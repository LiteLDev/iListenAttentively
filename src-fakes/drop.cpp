#include <cmath>
#include <ll/api/memory/Hook.h>
#include <mc/deps/core/math/Vec2.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/ecs/WeakEntityRef.h>
#include <mc/deps/shared_types/legacy/actor/ActorLocation.h>
#include <mc/deps/vanilla_components/StateVectorComponent.h>
#include <mc/entity/components/ActorRotationComponent.h>
#include <mc/entity/components/MovementInterpolatorComponent.h>
#include <mc/util/Random.h>
#include <mc/world/ContainerID.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorSwingSource.h>
#include <mc/world/actor/item/ItemActor.h>
#include <mc/world/actor/player/AbilitiesIndex.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/events/ActorDroppedItemEvent.h>
#include <mc/world/events/ActorGameplayEvent.h>
#include <mc/world/events/EventRef.h>
#include <mc/world/events/PlayerDropItemEvent.h>
#include <mc/world/events/PlayerEventCoordinator.h>
#include <mc/world/events/PlayerGameplayEvent.h>
#include <mc/world/inventory/transaction/InventoryAction.h>
#include <mc/world/inventory/transaction/InventorySource.h>
#include <mc/world/inventory/transaction/InventorySourceType.h>
#include <mc/world/item/ItemInstance.h>
#include <mc/world/item/ItemStack.h>
#include <mc/world/item/ItemStackBase.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/Spawner.h>
#include <numbers>

ActorDroppedItemEvent::ActorDroppedItemEvent() = default;

class ActorEventCoordinator {
public:
    void sendEvent(EventRef<ActorGameplayEvent<void>> const& event);
};

class ServerPlayerEventCoordinator : public PlayerEventCoordinator {};

namespace ila::fakes {

LL_AUTO_TYPE_INSTANCE_HOOK(
    ActorDropHook,
    HookPriority::Normal,
    Actor,
    &Actor::_drop,
    ItemActor const*,
    ItemStack const& item,
    bool const       randomly
) {
    if (!item.mValid_DeprecatedSeeComment || item.isNull() || item.mCount == 0 || !hasDimension()) {
        return nullptr;
    }

    Vec2 rotation = mBuiltInComponents->mActorRotationComponent->mRotationDegree;
    if (auto interpolator = getEntityContext().tryGetComponent<MovementInterpolatorComponent>();
        interpolator
        && (interpolator->mPositionSteps != 0 || interpolator->mRotationSteps != 0
            || interpolator->mHeadYawSteps != 0)) {
        rotation = interpolator->mRot;
    }

    Vec3 pos  = getAttachPos(SharedTypes::Legacy::ActorLocation::Eyes, 0.0f);
    pos.y    -= 0.5f;

    ItemActor* droppedItem = getLevel().getSpawner().spawnItem(getDimensionBlockSource(), item, this, pos, 40);
    if (droppedItem == nullptr) {
        return nullptr;
    }

    if (!mRemoved) {
        droppedItem->setOwner(getOrCreateUniqueID());
    }

    Random& random = getLevel().getThreadRandom();
    Vec3    velocity;
    if (randomly) {
        float const radius = random.nextFloat() * 0.5f;
        float const angle  = random.nextFloat() * (std::numbers::pi_v<float> * 2.0f);
        velocity           = Vec3{-std::sin(angle) * radius, 0.2f, std::cos(angle) * radius};
    } else {
        float const pitch = rotation.x * (std::numbers::pi_v<float> / 180.0f);
        float const yaw   = rotation.y * (std::numbers::pi_v<float> / 180.0f);

        velocity = Vec3{
            -std::cos(pitch) * std::sin(yaw) * 0.3f,
            -std::sin(pitch) * 0.3f,
            std::cos(pitch) * std::cos(yaw) * 0.3f,
        };

        float const angle      = random.nextFloat() * (std::numbers::pi_v<float> * 2.0f);
        float const magnitude  = random.nextFloat() * 0.02f;
        velocity.x            += std::cos(angle) * magnitude;
        velocity.z            += std::sin(angle) * magnitude;
    }

    droppedItem->mBuiltInComponents->mActorRotationComponent->mRotationDegree = rotation;
    droppedItem->mBuiltInComponents->mStateVectorComponent->mPosDelta         = velocity;
    droppedItem->throwTime()                                                  = 10;

    ActorDroppedItemEvent event{};
    event.mEntity                           = WeakEntityRef{getWeakEntity()};
    event.mItem                             = ItemInstance{item};
    ActorDroppedItemEvent const& eventConst = event;
    getLevel().getActorEventCoordinator().sendEvent(EventRef<ActorGameplayEvent<void>>(eventConst));
    return droppedItem;
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    ActorDropTowardsHook,
    HookPriority::Normal,
    Actor,
    &Actor::dropTowards,
    void,
    ItemStack const& item,
    Vec3             towards
) {
    if (!hasDimension()) {
        return;
    }

    Vec3 const attachPos = getAttachPos(SharedTypes::Legacy::ActorLocation::Count, 0.0f);
    Vec3 const spawnPos  = attachPos + mBuiltInComponents->mStateVectorComponent->mPosDelta;

    ItemActor* droppedItem = getLevel().getSpawner().spawnItem(getDimensionBlockSource(), item, nullptr, spawnPos, 40);
    if (droppedItem == nullptr) {
        return;
    }

    Vec2 rotation = mBuiltInComponents->mActorRotationComponent->mRotationDegree;
    if (auto interpolator = getEntityContext().tryGetComponent<MovementInterpolatorComponent>();
        interpolator
        && (interpolator->mPositionSteps != 0 || interpolator->mRotationSteps != 0
            || interpolator->mHeadYawSteps != 0)) {
        rotation = interpolator->mRot;
    }

    Vec3 velocity                                                              = (towards - attachPos) * 0.1f;
    velocity.y                                                                += 0.2f;
    droppedItem->mBuiltInComponents->mActorRotationComponent->mRotationDegree  = rotation;
    droppedItem->mBuiltInComponents->mStateVectorComponent->mPosDelta          = velocity;
    droppedItem->throwTime()                                                   = 10;

    ActorDroppedItemEvent event{};
    event.mEntity                           = WeakEntityRef{getWeakEntity()};
    event.mItem                             = ItemInstance{item};
    ActorDroppedItemEvent const& eventConst = event;
    getLevel().getActorEventCoordinator().sendEvent(EventRef<ActorGameplayEvent<void>>(eventConst));
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    PlayerDropHook,
    HookPriority::Normal,
    Player,
    &Player::$drop,
    bool,
    ItemStack const& item,
    bool const       randomly
) {
    constexpr AbilitiesIndex kManualDropAbility = static_cast<AbilitiesIndex>(2);

    if (!hasDimension()) {
        return false;
    }

    if (!mItemInUse->mItem->isNull()) {
        stopUsingItem();
    }

    if (!canUseAbility(kManualDropAbility) && getHealth() > 0) {
        return false;
    }

    if (!item.mValid_DeprecatedSeeComment) {
        return false;
    }

    if (!static_cast<bool>(item.mItem) && !static_cast<bool>(item.getBlockType())) {
        return false;
    }

    swing(ActorSwingSource::DropItem);
    InventorySource source{
        InventorySourceType::WorldInteraction,
        ContainerID::None,
        randomly ? InventorySource::InventorySourceFlags::WorldInteractionRandom
                 : InventorySource::InventorySourceFlags::NoFlag,
    };
    InventoryAction action{source, 0, ItemStack::EMPTY_ITEM(), item};
    mTransactionManager->addAction(action, false);

    if (!getLevel().isClientSide()) {
        auto* droppedItem = const_cast<ItemActor*>(Actor::_drop(item, randomly));
        if (droppedItem == nullptr) {
            return false;
        }

        PlayerDropItemEvent event{};
        event.mPlayer                         = getWeakEntity();
        event.mSpawnedItemActor               = droppedItem->getWeakEntity();
        PlayerDropItemEvent const& eventConst = event;
        static_cast<PlayerEventCoordinator&>(getLevel().getServerPlayerEventCoordinator())
            .sendEvent(EventRef<PlayerGameplayEvent<void>>(eventConst));
        return true;
    }

    ActorDroppedItemEvent event{};
    event.mEntity                           = WeakEntityRef{getWeakEntity()};
    event.mItem                             = ItemInstance{item};
    ActorDroppedItemEvent const& eventConst = event;
    getLevel().getActorEventCoordinator().sendEvent(EventRef<ActorGameplayEvent<void>>(eventConst));
    return true;
}

} // namespace ila::fakes
