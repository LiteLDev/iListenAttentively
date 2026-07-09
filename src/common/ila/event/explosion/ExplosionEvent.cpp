// clang-format off
#include "ila/utils/EventUtils.i.h"
#pragma include_alias("mc/world/level/block/ResourceDropsContext.h", "patch_mc/world/level/block/ResourceDropsContext.i.h")
#pragma include_alias(<mc/world/level/block/ResourceDropsContext.h>, <patch_mc/world/level/block/ResourceDropsContext.i.h>)
#pragma include_alias("mc/network/packet/LevelEventGenericPacketPayload.h", "patch_mc/network/packet/LevelEventGenericPacketPayload.i.h")
#pragma include_alias(<mc/network/packet/LevelEventGenericPacketPayload.h>, <patch_mc/network/packet/LevelEventGenericPacketPayload.i.h>)
// clang-format on
#include "ila/event/explosion/ExplosionEvent.h"
#include "ila/base/Gloabl.i.h"
#include "ila/event/explosion/ExplosionCollisionOffsetEvent.h"
#include "ila/event/explosion/ExplosionDamageEntityEvent.h"
#include "ila/event/explosion/ExplosionDestroyBlockEvent.h"
#include "ila/event/explosion/ExplosionExperienceBlockEvent.h"
#include "ila/event/explosion/ExplosionFlameEvent.h"
#include "ila/event/explosion/ExplosionKnockbackEntityEvent.h"
#include "ila/event/explosion/ExplosionLootBlockEvent.h"
#include "ila/event/explosion/ExplosionParticleEvent.h"
#include "ila/event/explosion/ExplosionProcessBlockEvent.h"
#include "ila/event/explosion/ExplosionProcessEntityEvent.h"
#include "ila/event/explosion/ExplosionSoundEvent.h"
#include "patch_mc/deps/shared_types/legacy/LevelEvent.i.h"
#include "patch_mc/deps/shared_types/legacy/LevelSoundEvent.i.h"
#include "patch_mc/world/events/EventCoordinator.i.h"
#include <algorithm>
#include <array>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstdlib>
#include <ll/api/base/Containers.h>
#include <ll/api/base/StdInt.h>
#include <ll/api/event/world/WorldEvent.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>
#include <mc/deps/core/math/IRandom.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/core/utility/optional_ref.h>
#include <mc/deps/ecs/gamerefs_entity/EntityContext.h>
#include <mc/deps/ecs/gamerefs_entity/GameRefsEntity.h>
#include <mc/deps/game_refs/WeakRef.h>
#include <mc/deps/shared_types/legacy/LevelEvent.h>
#include <mc/deps/shared_types/legacy/LevelSoundEvent.h>
#include <mc/deps/shared_types/legacy/actor/ActorDamageCause.h>
#include <mc/entity/components/ServerMovement.h>
#include <mc/entity/components/PostImpulseFallDamagePreventionComponent.h>
#include <mc/deps/nbt/CompoundTag.h>
#include <mc/entity/components_json_legacy/ExplodeComponent.h>
#include <mc/gameplayhandlers/CoordinatorResult.h>
#include <mc/network/packet/LevelEventGenericPacket.h>
#include <mc/util/Random.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorCategory.h>
#include <mc/world/actor/ActorDamageByActorSource.h>
#include <mc/world/actor/ActorDamageByChildActorSource.h>
#include <mc/world/actor/ActorDamageSource.h>
#include <mc/world/actor/ActorHurtResult.h>
#include <mc/world/actor/ActorSoundIdentifier.h>
#include <mc/world/actor/ActorType.h>
#include <mc/world/actor/KnockbackRules.h>
#include <mc/world/actor/item/ExperienceOrb.h>
#include <mc/world/attribute/Attribute.h>
#include <mc/world/attribute/AttributeInstance.h>
#include <mc/world/attribute/AttributeInstanceConstRef.h>
#include <mc/world/attribute/SharedAttributes.h>
#include <mc/world/events/BlockEventCoordinator.h>
#include <mc/world/events/EventRef.h>
#include <mc/world/events/ExplosionStartedEvent.h>
#include <mc/world/events/MutableBlockGameplayEvent.h>
#include <mc/world/events/gameevents/GameEventRegistry.h>
#include <mc/world/item/ItemStack.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/Explosion.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/ParticlesBlockExplosionEvent.h>
#include <mc/world/level/block/ActorChangeContext.h>
#include <mc/world/level/block/BedrockBlockNames.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/BlockChangeContext.h>
#include <mc/world/level/block/BlockType.h>
#include <mc/world/level/block/ResourceDrops.h>
#include <mc/world/level/block/ResourceDropsCause.h>
#include <mc/world/level/block/ResourceDropsContext.h>
#include <mc/world/level/block/VanillaBlockTypeIds.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/level/levelgen/structure/BoundingBox.h>
#include <mc/world/level/material/Material.h>
#include <mc/world/level/storage/GameRuleId.h>
#include <mc/world/level/storage/GameRules.h>
#include <optional>
#include <ranges>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ila::explosion {

struct ExplosionReflection {
    Vec3                                 pos;
    float                                radius;
    std::unordered_set<BlockPos>         $affected_blocks;
    bool                                 fire;
    bool                                 breaking;
    bool                                 allow_underwater;
    bool                                 can_toggle_blocks;
    float                                damage_scaling;
    bool                                 ignore_block_explosion_resistance;
    SharedTypes::Legacy::LevelEvent      particle_type;
    SharedTypes::Legacy::LevelSoundEvent sound_explosion_type;
    int64                                source_id;
    uintptr_t                            $region;
    float                                max_resistance;
    std::optional<bool>                  in_water_override;
    std::optional<int>                   total_damage_override;
    float                                knockback_scaling;
};

static_assert(
    sizeof(Explosion) == sizeof(ExplosionReflection),
    "Explosion and ExplosionReflection are not the same size"
);
static_assert(
    alignof(Explosion) == alignof(ExplosionReflection),
    "Explosion and ExplosionReflection are not the same alignment"
);

void ExplosionEvent::serialize(CompoundTag& nbt) const {
    WorldEvent::serialize(nbt);
    ll::reflection::serialize_to(nbt["explosion"], reinterpret_cast<ExplosionReflection&>(mExplosion)).value();
}

void ExplosionEvent::deserialize(CompoundTag const& nbt) {
    WorldEvent::deserialize(nbt);
    ll::reflection::deserialize(reinterpret_cast<ExplosionReflection&>(mExplosion), nbt["explosion"]).value();
}

LL_TYPE_INSTANCE_HOOK(ExplosionEventHook, HookPriority::Low, Explosion, &Explosion::explode, bool, IRandom& random) {
    if (mRadius == 0.0f) return false;

    if (eventPromise(ExplodingEvent{*this}).publish()) return false;

    auto& level   = mRegion.mLevel;
    auto  source  = optional_ref{level.fetchEntity(mSourceID, false)};
    auto  inWater = [&]() {
        if (*mInWaterOverride) return **mInWaterOverride;
        if (source) return source->isInWater();
        return mRegion.getLiquidBlock(BlockPos{mPos}).mBlockType->mMaterial.mType == SharedTypes::v1_26_20::MaterialType::Water;
    }();
    auto& airBlock       = *Block::tryGetFromRegistry(BedrockBlockNames::Air());
    auto& fireBlock      = *Block::tryGetFromRegistry(VanillaBlockTypeIds::Fire());
    auto  forceFullDrops = [&]() {
        if (!source) return false;
        if (auto type = source->getEntityTypeId(); type == ActorType::PrimedTnt || type == ActorType::MinecartTNT) {
            if (auto& gameRules = level.getGameRules(); gameRules.getBool(
                    GameRuleId{std::to_underlying(GameRules::GameRulesIndex::TntExplosionDropDecay)},
                    false
                )) {
                return true;
            }
        }
        return false;
    }();

    // 碰撞偏移（使用手动归一化代替 Vec3::normalized）
    if (auto& block = mRegion.getBlock(BlockPos{mPos}); block.mBlockType->mMaterial.mSolid && source) {
        auto posDelta = *source->mBuiltInComponents->mStateVectorComponent->mPosDelta;
        if (auto numSteps = std::min(static_cast<int>(std::ceil(posDelta.length())), 5); numSteps > 0) {
            Vec3 normalizedDelta;
            if (auto length = posDelta.length(); length >= 0.000099999997f) {
                normalizedDelta = posDelta / length;
            } else {
                normalizedDelta = {};
            }
            Vec3 currentPos = mPos;

            if (std::ranges::any_of(std::views::iota(0, numSteps), [&](auto) {
                currentPos -= normalizedDelta;
                return !mRegion.getBlock({currentPos}).mBlockType->mMaterial.mSolid;
            })) {
                eventPromise(ExplosionCollidingEvent{*this, mPos, currentPos})
                    .onSuccess([&] {
                    std::swap(*mPos, currentPos);
                    inWater =
                        mRegion.getLiquidBlock(BlockPos{currentPos}).mBlockType->mMaterial.mType == SharedTypes::v1_26_20::MaterialType::Water;
                })
                    .onSuccessEvent(ExplosionCollidedEvent{*this, currentPos, mPos})
                    .publish();
            }
        }
    }

    // 收集受影响方块
    if ((mBreaking || mCanToggleBlocks) && (!inWater || mAllowUnderwater)) {
        constexpr float stepScale = 2.0f / 15.0f; // 0.13333334f

        for (auto boundaryPos : BoundingBox{0, 16}.forEachPos()) {
            if (std::ranges::all_of(reinterpret_cast<std::array<int, 3>&>(boundaryPos), [](auto value) {
                return value != 0 && value != 15;
            })) {
                continue;
            }

            auto direction  = Vec3{boundaryPos} * stepScale - 1.0f;
            direction      /= direction.length();

            Vec3 currentPos = mPos;

            for (auto remainingStrength  = mRadius * (random.nextFloat() * 0.6f + 0.7f); remainingStrength > 0.0f;
                 remainingStrength      -= 0.75f * 0.3f) {
                BlockPos blockPos(currentPos);

                if (!mAllowUnderwater) {
                    if (auto& extraBlock = mRegion.getExtraBlock(blockPos);
                        extraBlock.mBlockType->mMaterial.mType == SharedTypes::v1_26_20::MaterialType::Water) {
                        remainingStrength = 0.0f;
                    }
                }

                auto& currentBlock = mRegion.getBlock(blockPos);

                if (currentBlock.isAir()
                    || (mAllowUnderwater ? currentBlock.mBlockType->mMaterial.mType == SharedTypes::v1_26_20::MaterialType::Water : false)) {
                    if (mFire && mRegion.getBlock(blockPos.add({0, -1, 0})).mBlockType->mMaterial.mSolid) {
                        mAffectedBlocks->emplace(blockPos);
                    }
                } else {
                    auto blockResistance =
                        mIgnoreBlockExplosionResistance ? 0.0f : currentBlock.mDirectData->mExplosionResistance;
                    if (source && source->canDestroyBlock(currentBlock)) {
                        blockResistance = std::min(mMaxResistance * 0.2f, blockResistance);
                    }
                    remainingStrength -= (blockResistance * 0.3f + 0.3f) * 0.3f;
                    if (remainingStrength > 0.0f) mAffectedBlocks->emplace(blockPos);
                }

                currentPos += direction * 0.3f;
            }
        }
    }

    // 脚本层推送事件
    if (!level.isClientSide()) {
        struct {
            std::unordered_set<BlockPos> mBlocks;
            Dimension&                   mDimension;
            WeakRef<EntityContext>       mSource;
        } event{mAffectedBlocks, mRegion.mDimension, source ? source->mEntityContext->getWeakRef() : WeakRef<EntityContext>{}};
        static_assert(sizeof(event) == sizeof(ExplosionStartedEvent));
        EventRef<MutableBlockGameplayEvent<CoordinatorResult>> eventRef{
            reinterpret_cast<ExplosionStartedEvent&>(event)
        };
        if (level.getBlockEventCoordinator().sendEvent(eventRef) == CoordinatorResult::Cancel) return false;
        mAffectedBlocks = std::move(event.mBlocks);
    }

    auto doubleRadius = mRadius * 2.0f;
    auto sourcePos = source.transform([&](Actor& entity) {
        auto type = entity.getEntityTypeId();
        return type == ActorType::WindChargeProjectile || type == ActorType::BreezeWindChargeProjectile ? *mPos : entity.getEyePos();
    }).value_or(mPos);

    // 处理实体
    for (auto entity : _getActorsInRange(source, doubleRadius)) {
        if (entity->isSpectator() || eventPromise(ExplosionProcessEntityingEvent{*this, *entity}).publish()) {
            continue;
        }

        auto& aabb      = *entity->mBuiltInComponents->mAABBShapeComponent->mAABB;
        auto  entityPos = *entity->mBuiltInComponents->mStateVectorComponent->mPos;
        entityPos.y     = aabb.min.y;
        auto distance   = static_cast<float>((entityPos - sourcePos).length());

        if (entity->getEntityTypeId() != ActorType::Player) {
            distance = std::max(0.0f, distance - entity->mBuiltInComponents->mAABBShapeComponent->mBBDim->x * 0.5f);
        }

        auto normalizedDistance = distance / doubleRadius;
        if (normalizedDistance > 1.0f) continue;
        auto pow         = (1.0f - normalizedDistance) * mRegion.getSeenPercent(mPos, aabb) * mKnockbackScaling;
        auto totalDamage = *mTotalDamageOverride
                               ? static_cast<float>(**mTotalDamageOverride)
                               : ((pow * pow * 3.5f + pow * 0.5f * 7.0f) * doubleRadius + 1.0f) * mDamageScaling;

        // 伤害实体
        {
            auto [damageSource, damaging, knockback] = [&]() -> std::tuple<ActorDamageSource, bool, bool> {
                using ActorDamageCause = SharedTypes::Legacy::ActorDamageCause;

                if (entity->isBlocking()) {
                    pow *= 0.5f;
                    ActorDamageCause cause = ActorDamageCause::BlockExplosion;
                    if (source && (source->hasCategory(ActorCategory::Mob) || source->getEntityTypeId() == ActorType::PrimedTnt)) {
                        cause = ActorDamageCause::EntityExplosion;
                    }
                    ActorDamageSource ds;
                    ds.mCause = cause;
                    return {ds, false, false};
                }

                if (std::abs(mDamageScaling) <= 0.0000099999997f) {
                    return {};
                }

                if (!source) {
                    ActorDamageSource ds;
                    ds.mCause = ActorDamageCause::BlockExplosion;
                    return {ds, true, false};
                }

                if (inWater && !mAllowUnderwater) {
                    totalDamage = 0.0f;
                }

                if (auto* childSource = level.fetchEntity(source->getSourceUniqueID(), false); childSource) {
                    return {
                        ActorDamageByChildActorSource{*source, *childSource, ActorDamageCause::EntityExplosion},
                        true,
                        false
                    };
                }

                if (source->hasCategory(ActorCategory::Mob) || source->getEntityTypeId() == ActorType::PrimedTnt) {
                    return {
                        ActorDamageByActorSource{*source, ActorDamageCause::EntityExplosion},
                        true,
                        false
                    };
                }

                ActorDamageSource ds;
                ds.mCause = ActorDamageCause::BlockExplosion;
                return {ds, true, false};
            }();

            if (!eventPromise(
                    ExplosionDamageEntityingEvent{*this, *entity, damageSource, totalDamage, damaging, knockback}
                ).publish() && totalDamage > 0.0f
            ) {
                entity->hurt(damageSource, totalDamage, damaging, knockback);
                eventPromise(
                    ExplosionDamageEntityedEvent{*this, *entity, damageSource, totalDamage, damaging, knockback}
                ).publish();
            }
        }

        // 击退实体（使用 isKnockbackResistant）
        if (!KnockbackRules::isKnockbackResistant(*entity)) {
            auto type              = entity->getEntityTypeId();
            auto actorKnockbackPos = type == ActorType::PrimedTnt
                                         ? *entity->mBuiltInComponents->mStateVectorComponent->mPos
                                         : entity->getEyePos();

            auto direction = actorKnockbackPos - mPos;
            if (auto directionLength = direction.length(); directionLength >= 0.0001f) {
                direction *= 1.0f / directionLength;
            } else {
                direction = {};
            }

            Vec3 knockback = direction * KnockbackRules::getScaledKnockbackForce(*entity, pow);

            eventPromise(ExplosionKnockbackEntityingEvent{*this, *entity, knockback}).onCancel([&] {
                knockback = {};
            }).publish();

            if (knockback.x != 0.0f || knockback.y != 0.0f || knockback.z != 0.0f) {
                if (type == ActorType::Player && source) {
                    auto explodeComp = source->mEntityContext->tryGetComponent<ExplodeComponent>();
                    if (explodeComp && explodeComp->mNegatesFallDamage) {
                        auto& windComp = entity->mEntityContext->getOrAddComponent<PostImpulseFallDamagePreventionComponent>();
                        windComp.mKnockbackStartYCoordinate =
                            entity->mBuiltInComponents->mStateVectorComponent->mPos->y;
                        windComp.mTicksAfterAddition = 0;
                    }
                }

                *entity->mBuiltInComponents->mStateVectorComponent->mPosDelta += knockback;
                ServerMovement::notifyOfServerInitiatedMotion(entity->mEntityContext);

                eventPromise(ExplosionKnockbackEntityedEvent{*this, *entity, knockback}).publish();
            }
        }

        eventPromise(ExplosionProcessEntityedEvent{*this, *entity}).publish();
    }

    // 粒子效果和声音
    if (mRadius > 0.0f) {
        eventPromise(ExplosionSoundingEvent{*this})
            .onSuccess([&] { level.broadcastSoundEvent(mRegion, mSoundExplosionType, mPos, -1, {}, false, std::nullopt); })
            .onSuccessEvent(ExplosionSoundedEvent{*this})
            .publish();
        eventPromise(ExplosionParticlingEvent{*this, mPos, mParticleType})
            .onSuccess([&] { level.broadcastLocalEvent(mRegion, mParticleType, mPos, static_cast<int>(mRadius)); })
            .onSuccessEvent(ExplosionParticledEvent{*this, mPos, mParticleType})
            .publish();
    }

    mRegion.postGameEvent(source, GameEventRegistry::explode(), mPos, nullptr);

    BlockPos minPos{INT_MAX};
    BlockPos maxPos{INT_MIN};
    bool     blockChangable{false};

    // 方块处理
    {
        ParticlesBlockExplosionEvent particlesEvent{mRadius, mPos, {}};
        ll::OrderedMap<std::pair<BlockPos, bool>, std::pair<Block const*, std::vector<ItemStack>>> blockDrops;

        for (auto& blockPos : *mAffectedBlocks) {
            auto& block      = mRegion.getBlock(blockPos);
            bool  isAirBlock = block.isAir();
            auto& extraBlock = mRegion.getExtraBlock(blockPos);

            if (!level.isClientSide() && mCanToggleBlocks) {
                if (!source || source->getEntityTypeId() != ActorType::BreezeWindChargeProjectile) {
                    block.mBlockType->_onHitByActivatingAttack(mRegion, blockPos, source);
                }
            }

            if (mBreaking) {
                if (!random.nextInt(8)) {
                    auto pos = Vec3{blockPos};
                    eventPromise(
                        ExplosionParticlingEvent{*this, pos, SharedTypes::Legacy::LevelEvent::ParticlesBlockExplosion}
                    )
                        .onSuccess([&] {
                        particlesEvent.mPositions->emplace_back(pos);
                    }).publish();
                }

                if (!block.isAir()) {
                    if (!level.isClientSide()) {
                        ResourceDropsContext resourceDropsContext{
                            ResourceDropsCause::Explosion,
                            forceFullDrops ? 1.0f : mRadius,
                            ItemStack::EMPTY_ITEM(),
                            blockPos,
                            mRegion.mDimension.mId,
                            mRegion
                        };

                        auto destroy = [&, &random = level.getRandom()](Block const& block, bool isExtraBlock) {
                            if (block.isAir() || eventPromise(ExplosionProcessBlockingEvent{*this, blockPos, block, isExtraBlock}).publish()) return;
                            auto resources = block.mBlockType->getResourceDrops(block, random, resourceDropsContext);
                            {
                                auto event = !eventPromise(
                                    ExplosionLootingBlockEvent{
                                        *this,
                                        blockPos,
                                        block,
                                        isExtraBlock,
                                        *resources.mItems
                                    }
                                ).publish();
                                if (!event && !resources.mItems->empty()) {
                                    blockDrops.insert({
                                        {blockPos, isExtraBlock                },
                                        {&block,   std::move(*resources.mItems)}
                                    });
                                }
                            }
                            {
                                auto event = eventPromise(
                                    ExplosionExperienceBlockingEvent{
                                        *this,
                                        blockPos,
                                        block,
                                        isExtraBlock,
                                        resources.mExperienceCount
                                    }
                                ).publish();
                                if (!event && resources.mExperienceCount > 0) {
                                    ExperienceOrb::spawnOrbs(
                                        mRegion,
                                        blockPos,
                                        resources.mExperienceCount,
                                        ExperienceOrb::DropType::FromBlock,
                                        nullptr
                                    );

                                    eventPromise(
                                        ExplosionExperienceBlockedEvent{
                                            *this,
                                            blockPos,
                                            block,
                                            isExtraBlock,
                                            resources.mExperienceCount
                                        }
                                    ).publish();
                                }
                            }
                            {
                                if (!eventPromise(ExplosionDestroyBlockingEvent{*this, blockPos, block, isExtraBlock}).publish()) {
                                    if (isExtraBlock) {
                                        mRegion.setExtraBlock(blockPos, airBlock, 3);
                                    } else {
                                        BlockChangeContext changeCtx{};
                                        changeCtx.mContextSource = ActorChangeContext{source};
                                        mRegion.setBlock(blockPos, airBlock, 3, nullptr, changeCtx);
                                        isAirBlock = true;
                                    }
                                    block.mBlockType->spawnAfterBreak(mRegion, block, blockPos, resourceDropsContext);
                                    eventPromise(
                                        ExplosionDestroyBlockedEvent{*this, blockPos, block, isExtraBlock}
                                    ).publish();
                                }
                            }
                            block.mBlockType->onExploded(mRegion, blockPos, source);
                            eventPromise(ExplosionProcessBlockedEvent{*this, blockPos, block, isExtraBlock}).publish();
                        };

                        destroy(block, false);
                        destroy(extraBlock, true);
                    }

                    minPos         = std::min(minPos, blockPos);
                    maxPos         = std::max(maxPos, blockPos);
                    blockChangable = true;
                }

                if (!level.isClientSide()) {
                    for (auto* listener : level.getBlockEventCoordinator().mListeners) {
                        if (!listener) continue;
                        listener->onBlockExploded(mRegion.mDimension, blockPos, block, source);
                    }
                }
            }

            if (!level.isClientSide()) {
                LevelEventGenericPacket{
                    {std::to_underlying(SharedTypes::Legacy::LevelEvent::ParticlesBlockExplosion),
                     std::move(*particlesEvent.save())}
                }.sendTo(*mPos, mRegion.mDimension.mId);
                for (auto& pos : *particlesEvent.mPositions) {
                    eventPromise(
                        ExplosionParticledEvent{*this, pos, SharedTypes::Legacy::LevelEvent::ParticlesBlockExplosion}
                    )
                        .publish();
                }
            }

            if (mFire && isAirBlock) {
                if (mRegion.getBlock(blockPos.add({0, -1, 0})).mBlockType->mMaterial.mSolid && !random.nextInt(3)) {
                    auto pos   = blockPos;
                    auto event = ExplosionFlamingEvent{*this, pos};
                    if (!event.isCancelled()) {
                        BlockChangeContext changeCtx{};
                        changeCtx.mContextSource = ActorChangeContext{source};
                        mRegion.setBlock(pos, fireBlock, 3, nullptr, changeCtx);
                        eventPromise(ExplosionFlamedEvent{*this, pos}).publish();
                    }

                    minPos = std::min(minPos, pos);
                    maxPos = std::max(maxPos, pos);
                }
            }
        }

        // 生成方块掉落物
        if (!blockDrops.empty()) {
            std::vector<std::pair<ItemStack, BlockPos>> items;
            for (auto& [key, value] : blockDrops) {
                for (auto& itemStack : value.second) {
                    _addOrMergeItemStack(itemStack, key.first, items);
                }
            }
            for (auto& [itemStack, pos] : items) {
                BlockType::popResource(mRegion, pos, itemStack);
            }
            for (auto& [key, value] : blockDrops) {
                eventPromise(ExplosionLootedBlockEvent{*this, key.first, *value.first, key.second, value.second})
                    .publish();
            }
        }
    }

    // 方块更新（新版使用 fireAreaChanged）
    if (blockChangable) {
        mRegion.fireAreaChanged(minPos.add(-1), maxPos.add(1));
    }

    eventPromise(ExplodedEvent{*this}).publish();
    return true;
}

EventHook(ExplodingEvent, ExplodedEvent, <ExplosionEventHook>);
EventHook(ExplosionCollidingEvent, ExplosionCollidedEvent, <ExplosionEventHook>);
EventHook(ExplosionDamageEntityingEvent, ExplosionDamageEntityedEvent, <ExplosionEventHook>);
EventHook(ExplosionDestroyBlockingEvent, ExplosionDestroyBlockedEvent, <ExplosionEventHook>);
EventHook(ExplosionExperienceBlockingEvent, ExplosionExperienceBlockedEvent, <ExplosionEventHook>);
EventHook(ExplosionFlamingEvent, ExplosionFlamedEvent, <ExplosionEventHook>);
EventHook(ExplosionKnockbackEntityingEvent, ExplosionKnockbackEntityedEvent, <ExplosionEventHook>);
EventHook(ExplosionLootingBlockEvent, ExplosionLootedBlockEvent, <ExplosionEventHook>);
EventHook(ExplosionParticlingEvent, ExplosionParticledEvent, <ExplosionEventHook>);
EventHook(ExplosionProcessBlockingEvent, ExplosionProcessBlockedEvent, <ExplosionEventHook>);
EventHook(ExplosionProcessEntityingEvent, ExplosionProcessEntityedEvent, <ExplosionEventHook>);
EventHook(ExplosionSoundingEvent, ExplosionSoundedEvent, <ExplosionEventHook>);

} // namespace ila::explosion

WindChargeImpulse::WindChargeImpulse() : mUnk75c12e() { mUnk75c12e.as<bool>() = false; }
WindChargeImpulse::WindChargeImpulse(WindChargeImpulse const&)            = default;
WindChargeImpulse& WindChargeImpulse::operator=(WindChargeImpulse const&) = default;