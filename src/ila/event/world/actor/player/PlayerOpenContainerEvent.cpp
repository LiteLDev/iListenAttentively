#include "ila/event/world/actor/player/PlayerOpenContainerEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/deps/ecs/WeakEntityRef.h>
#include <mc/deps/shared_types/legacy/ContainerType.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/network/packet/ContainerOpenPacket.h>
#include <mc/server/module/VanillaServerGameplayEventListener.h>
#include <mc/world/events/EventResult.h>
#include <mc/world/events/PlayerOpenContainerEvent.h>

namespace ila::mc::inline world::inline actor::inline player {

void PlayerOpenContainerBeforeEvent::serialize(CompoundTag& nbt) const {
    Cancellable::serialize(nbt);
    nbt["containerBlockPos"] = ListTag{containerBlockPos().x, containerBlockPos().y, containerBlockPos().z};
    nbt["containerType"]     = magic_enum::enum_name(containerType());
    nbt["containerActorId"]  = containerActorId().rawID;
}
void PlayerOpenContainerBeforeEvent::deserialize(CompoundTag const& nbt) {
    Cancellable::deserialize(nbt);
    containerBlockPos().x = nbt["containerBlockPos"][0];
    containerBlockPos().y = nbt["containerBlockPos"][1];
    containerBlockPos().z = nbt["containerBlockPos"][2];
    containerType() = magic_enum::enum_cast<SharedTypes::Legacy::ContainerType>(nbt["containerType"].get<StringTag>())
                          .value_or(containerType());
    containerActorId().rawID = nbt["containerActorId"];
}
BlockPos&                           PlayerOpenContainerBeforeEvent::containerBlockPos() const { return mPos; }
SharedTypes::Legacy::ContainerType& PlayerOpenContainerBeforeEvent::containerType() const { return mContainerType; }
ActorUniqueID& PlayerOpenContainerBeforeEvent::containerActorId() const { return mContainerActorId; }

void PlayerOpenContainerAfterEvent::serialize(CompoundTag& nbt) const {
    PlayerEvent::serialize(nbt);
    nbt["containerBlockPos"] = ListTag{containerBlockPos().x, containerBlockPos().y, containerBlockPos().z};
    nbt["containerType"]     = magic_enum::enum_name(containerType());
    nbt["containerActorId"]  = containerActorId().rawID;
}
BlockPos const&                           PlayerOpenContainerAfterEvent::containerBlockPos() const { return mPos; }
SharedTypes::Legacy::ContainerType const& PlayerOpenContainerAfterEvent::containerType() const {
    return mContainerType;
}
ActorUniqueID const& PlayerOpenContainerAfterEvent::containerActorId() const { return mContainerActorId; }

LL_TYPE_INSTANCE_HOOK(
    OpenContainerHook,
    HookPriority::Normal,
    VanillaServerGameplayEventListener,
    &VanillaServerGameplayEventListener::$onEvent,
    EventResult,
    PlayerOpenContainerEvent const& playerOpenContainerEvent
) {
    Actor* actor = playerOpenContainerEvent.mPlayer->tryUnwrap<Actor>();
    if (actor && actor->isType(ActorType::Player)) {
        Player& player      = *static_cast<Player*>(actor);
        auto    beforeEvent = PlayerOpenContainerBeforeEvent(
            player,
            const_cast<BlockPos&>(*playerOpenContainerEvent.mBlockPos),
            const_cast<SharedTypes::Legacy::ContainerType&>(playerOpenContainerEvent.mContainerType),
            const_cast<ActorUniqueID&>(*playerOpenContainerEvent.mEntityUniqueId)
        );
        LLEventBus.publish(beforeEvent);
        if (beforeEvent.isCancelled()) {
            return EventResult::StopProcessing;
        }
        auto result = origin(playerOpenContainerEvent);
        if (result == EventResult::KeepGoing) {
            LLEventBus.publish(PlayerOpenContainerAfterEvent(
                player,
                playerOpenContainerEvent.mBlockPos,
                playerOpenContainerEvent.mContainerType,
                playerOpenContainerEvent.mEntityUniqueId
            ));
        }
        return result;
    }
    return origin(playerOpenContainerEvent);
}

Event_Hook_Factory(PlayerOpenContainer, <OpenContainerHook>);

} // namespace ila::mc::inline world::inline actor::inline player
