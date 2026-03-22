#include <entt/container/dense_map.hpp>
#include <exception>
#include <ll/api/base/StdInt.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/core/utility/typeid_t.h>
#include <mc/deps/ecs/WeakEntityRef.h>
#include <mc/deps/ecs/gamerefs_entity/StackResultStorageEntity.h>
#include <mc/deps/ecs/gamerefs_entity/WeakStorageEntity.h>
#include <mc/deps/shared_types/legacy/actor/ActorDamageCause.h>
#include <mc/deps/vanilla_components/ActorDataDirtyFlagsComponent.h>
#include <mc/deps/vanilla_components/ActorDataFlagComponent.h>
#include <mc/deps/vanilla_components/ActorTypeComponent.h>
#include <mc/entity/components/ActorOwnerComponent.h>
#include <mc/entity/components/IsFishableFlagComponent.h>
#include <mc/entity/components/PassengerComponent.h>
#include <mc/entity/components/PhysicsComponent.h>
#include <mc/entity/definitions/ReflectProjectileDefinition.h>
#include <mc/entity/factory/DefinitionInstanceGroup.h>
#include <mc/entity/factory/IDefinitionInstance.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/network/SpatialActorNetworkData.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorDamageByActorSource.h>
#include <mc/world/actor/ActorDamageByChildActorSource.h>
#include <mc/world/actor/ActorDataIDs.h>
#include <mc/world/actor/ActorDefinitionDescriptor.h>
#include <mc/world/actor/ActorFlags.h>
#include <mc/world/actor/ActorType.h>
#include <mc/world/actor/FishingHook.h>
#include <mc/world/actor/SynchedActorDataEntityWrapper.h>
#include <mc/world/actor/provider/ActorCollision.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/ClipDefaults.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/ShapeType.h>
#include <mc/world/phys/AABB.h>
#include <mc/world/phys/AABBHitResult.h>
#include <mc/world/phys/HitResult.h>
#include <mc/world/phys/HitResultType.h>
#include <memory>
#include <vector>

#include "ila/base/Gloabl.i.h"
#include <fmt/format.h>
#include <ll/api/memory/Hook.h>
#include <iostream>

template <typename _DefinitionType>
[[nodiscard]] _DefinitionType const* tryGetDefinition(ActorDefinitionDescriptor* pDescriptor) {
    using Map = ::entt::dense_map<unsigned short, ::std::shared_ptr<::IDefinitionInstance>>;

    auto const& definitionMap = pDescriptor->mDefinitionGroup->mUnk4e28b4.as<Map>();
    auto const  typeId        = ::Bedrock::type_id<::IDefinitionInstance, _DefinitionType>();
    auto const  it            = definitionMap.find(typeId.value);
    if (it == definitionMap.end() || !it->second) {
        return nullptr;
    }

    return reinterpret_cast<_DefinitionType const*>(
        reinterpret_cast<char const*>(it->second.get()) + sizeof(::IDefinitionInstance)
    );
}

namespace ila::fakes {

LL_AUTO_TYPE_INSTANCE_HOOK(
    FishingHookHitCheckHook,
    HookPriority::Normal,
    FishingHook,
    &FishingHook::_hitCheck,
    HitResult
) {
    HitResult result;

    if (mInGround || (getFishingTarget() && getFishingTarget()->isAlive())) {
        return result;
    }

    Vec3 const from = getPosition();
    Vec3       to   = from + getPosDelta();
    std::cout << fmt::format("FishingHook::_hitCheck from {} to {}\n", from, to);
    result = getDimensionBlockSource().clip(
        from,
        to,
        false,
        ShapeType::Collision,
        200,
        false,
        false,
        nullptr,
        [](BlockSource const&, Block const&, bool) { return true; },
        false
    );
    if (result.mType != HitResultType::NoHit && result.mType != HitResultType::EntityOutOfRange) {
        to = result.mPos;
    }

    AABB       searchBox = getAABB();
    Vec3 const delta     = getPosDelta();
    if (delta.x < 0.0f) {
        searchBox.min.x += delta.x;
    } else if (delta.x > 0.0f) {
        searchBox.max.x += delta.x;
    }
    if (delta.y < 0.0f) {
        searchBox.min.y += delta.y;
    } else if (delta.y > 0.0f) {
        searchBox.max.y += delta.y;
    }
    if (delta.z < 0.0f) {
        searchBox.min.z += delta.z;
    } else if (delta.z > 0.0f) {
        searchBox.max.z += delta.z;
    }
    searchBox = searchBox.cloneAndGrow(1.0f);

    Actor* owner = getOwner();

    Actor* hitActor        = nullptr;
    float  nearestDistance = 0.0f;

    for (Actor* candidate : getDimensionBlockSource().fetchEntities(this, searchBox, false, false)) {
        if (!candidate->canInteractWithOtherEntitiesInGame()
            || !ActorCollision::isPickable(candidate->getEntityContext())
            || !candidate->getEntityContext().hasComponent<IsFishableFlagComponent>()
            || candidate == owner && mFlightTime < 5) {
            continue;
        }

        if (owner) {
            if (owner->isRiding() && candidate == owner->getVehicle()) {
                continue;
            }
            if (candidate->isRiding()) {
                auto passenger = candidate->getEntityContext().tryGetComponent<PassengerComponent>();
                if (passenger) {
                    auto const vehicleRawId = passenger->mVehicle->mEntity->mEntity->mRawId;
                    if ((vehicleRawId & 0x3FFFF) != 0x3FFFF) {
                        Actor* vehicle = candidate->getLevel().fetchEntity(passenger->mVehicle->mActorID, false);
                        if (vehicle == owner) {
                            continue;
                        }
                    }
                }
            }
        }

        if (ActorDefinitionDescriptor* descriptor = candidate->mCurrentDescription.get(); descriptor) {
            if (auto const* reflectProjectileDefinition = tryGetDefinition<ReflectProjectileDefinition>(descriptor)) {
                auto const projectileTypeComponent = getEntityContext().tryGetComponent<ActorTypeComponent>();
                if (!projectileTypeComponent) {
                    std::terminate();
                }
                if (projectileTypeComponent->mType != ActorType::Undefined) {
                    bool skipCandidate = false;
                    for (ActorType reflectedType :
                         reflectProjectileDefinition->mUnk9caa5a.as<std::vector<ActorType>>()) {
                        if (reflectedType == projectileTypeComponent->mType) {
                            skipCandidate = true;
                            break;
                        }
                    }
                    if (skipCandidate) {
                        continue;
                    }
                }
            }
        }

        AABBHitResult clipResult =
            AABB{candidate->getAABB().min - 0.30000001f, candidate->getAABB().max + 0.30000001f}.clip(from, to);
        if (!clipResult.mIsHit) {
            continue;
        }

        float const distance = (float)from.distanceToSqr(clipResult.mPos);
        if (nearestDistance != 0.0f && distance >= nearestDistance) {
            continue;
        }

        hitActor        = candidate;
        nearestDistance = distance;
    }

    if (hitActor && hitActor->isAlive()) {
        if (!getLevel().isClientSide()) {
            mEntityData->set((ushort)ActorDataIDs::Target, hitActor->getOrCreateUniqueID().rawID);
        }

        result = HitResult{from, to - from, *hitActor, hitActor->getPosition(), hitActor->getAABB()};

        Actor* hurtTarget = nullptr;
        {
            StackResultStorageEntity hitEntity{reinterpret_cast<WeakStorageEntity const&>(result.mEntity)};
            if (hitEntity) {
                if (auto ownerComponent = hitEntity->tryGetComponent<ActorOwnerComponent>(); ownerComponent) {
                    hurtTarget = ownerComponent->mActor.get();
                }
            }
        }

        if (Actor* source = getOwner(); source) {
            ActorDamageByChildActorSource damageSource{
                *this,
                *source,
                SharedTypes::Legacy::ActorDamageCause::Projectile
            };
            if (!hurtTarget) {
                std::terminate();
            }
            hurtTarget->hurt(damageSource, 0.0f, true, false);
        } else {
            ActorDamageByActorSource damageSource{*this, SharedTypes::Legacy::ActorDamageCause::Projectile};
            if (!hurtTarget) {
                std::terminate();
            }
            hurtTarget->hurt(damageSource, 0.0f, true, false);
        }

        mNetworkData->mUnkf6f1aa.as<bool>() = false;
        if (getEntityContext().hasComponent<PhysicsComponent>()) {
            auto& dirtyFlags = getEntityContext().getOrAddComponent<ActorDataDirtyFlagsComponent>();
            auto& actorFlags = getEntityContext().getOrAddComponent<ActorDataFlagComponent>();
            if (actorFlags.mValue.test(static_cast<size_t>(ActorFlags::HasCollision))) {
                actorFlags.mValue.reset(static_cast<size_t>(ActorFlags::HasCollision));
                dirtyFlags.mDirtyFlags->set(0);
            }
            auto persistentDirtyFlags = getEntityContext().tryGetComponent<ActorDataDirtyFlagsComponent>();
            auto persistentActorFlags = getEntityContext().tryGetComponent<ActorDataFlagComponent>();
            if (!persistentDirtyFlags || !persistentActorFlags) {
                std::terminate();
            }
            if (persistentActorFlags->mValue.test(static_cast<size_t>(ActorFlags::HasGravity))) {
                persistentActorFlags->mValue.reset(static_cast<size_t>(ActorFlags::HasGravity));
                persistentDirtyFlags->mDirtyFlags->set(0);
            }
        }
    }

    if (result.mType != HitResultType::NoHit && result.mType != HitResultType::EntityOutOfRange) {
        Actor*                   ownerActor = nullptr;
        StackResultStorageEntity hitEntity{reinterpret_cast<WeakStorageEntity const&>(result.mEntity)};
        if (hitEntity) {
            if (auto ownerComponent = hitEntity->tryGetComponent<ActorOwnerComponent>(); ownerComponent) {
                ownerActor = ownerComponent->mActor.get();
            }
        }
        mInGround = ownerActor == nullptr;
    }

    return result;
}

} // namespace ila::fakes