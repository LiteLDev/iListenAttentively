#include "ila/event/minecraft/actor/player/PlayerOpenContainerEvent.h"
#include "ila/base/Gloabl.h"
#include "ila/event/minecraft/server/SendPacketEvent.h"
#include <mc/legacy/ActorUniqueID.h>
#include <mc/network/NetworkBlockPosition.h>
#include <mc/network/packet/ContainerOpenPacket.h>

namespace ila::mc::inline world::inline actor::inline player
{

void PlayerOpenContainerBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["containerBlockPos"] =
        ListTag { containerBlockPos().x, containerBlockPos().y, containerBlockPos().z };
    nbt["containerId"]      = static_cast<schar>(containerId());
    nbt["containerType"]    = magic_enum::enum_name(containerType());
    nbt["containerActorId"] = containerActorId().rawID;
}
void PlayerOpenContainerBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    containerBlockPos().x = nbt["containerBlockPos"][0];
    containerBlockPos().y = nbt["containerBlockPos"][1];
    containerBlockPos().z = nbt["containerBlockPos"][2];
    containerId()         = static_cast<ContainerID>(nbt["containerId"].get<ByteTag>().data);
    containerType() =
        magic_enum::enum_cast<SharedTypes::Legacy::ContainerType>(nbt["containerType"].get<StringTag>())
            .value_or(containerType());
    containerActorId().rawID = nbt["containerActorId"];
}
BlockPos&    PlayerOpenContainerBeforeEvent::containerBlockPos() const { return mPos; }
ContainerID& PlayerOpenContainerBeforeEvent::containerId() const { return mContainerId; }
SharedTypes::Legacy::ContainerType& PlayerOpenContainerBeforeEvent::containerType() const
{
    return mContainerType;
}
ActorUniqueID& PlayerOpenContainerBeforeEvent::containerActorId() const { return mContainerActorId; }

void PlayerOpenContainerAfterEvent::serialize(CompoundTag& nbt) const
{
    ServerPlayerEvent::serialize(nbt);
    nbt["containerBlockPos"] =
        ListTag { containerBlockPos().x, containerBlockPos().y, containerBlockPos().z };
    nbt["containerId"]      = static_cast<schar>(containerId());
    nbt["containerType"]    = magic_enum::enum_name(containerType());
    nbt["containerActorId"] = containerActorId().rawID;
}
BlockPos const&    PlayerOpenContainerAfterEvent::containerBlockPos() const { return mPos; }
ContainerID const& PlayerOpenContainerAfterEvent::containerId() const { return mContainerId; }
SharedTypes::Legacy::ContainerType const& PlayerOpenContainerAfterEvent::containerType() const
{
    return mContainerType;
}
ActorUniqueID const& PlayerOpenContainerAfterEvent::containerActorId() const { return mContainerActorId; }

Event_Listener_Factory(PlayerOpenContainerBefore)
{
    nextTick([this]() -> void { // Prevent deadlock
        mListeners.emplace_back(
            LLEventBus.emplaceListener<ila::mc::server::SendPacketBeforeEvent<ContainerOpenPacket>>(
                [](ila::mc::server::SendPacketBeforeEvent<ContainerOpenPacket>& event) -> void {
                    if (auto player = event.player(); player)
                    {
                        auto& packet      = event.packet();
                        auto  beforeEvent = PlayerOpenContainerBeforeEvent(
                            *event.player(),
                            *packet.mPos,
                            packet.mContainerId,
                            packet.mType,
                            *packet.mEntityUniqueID
                        );
                        LLEventBus.publish(beforeEvent);
                        if (beforeEvent.isCancelled())
                        {
                            event.cancel();
                            event.player()->doDeleteContainerManager(false);
                        }
                    }
                }
            )
        );
    });
}

Event_Listener_Factory(PlayerOpenContainerAfter)
{
    nextTick([this]() -> void { // Prevent deadlock
        mListeners.emplace_back(
            LLEventBus.emplaceListener<ila::mc::server::SendPacketAfterEvent<ContainerOpenPacket>>(
                [](ila::mc::server::SendPacketAfterEvent<ContainerOpenPacket>& event) -> void {
                    if (auto player = event.player(); player)
                    {
                        auto& packet = event.packet();
                        LLEventBus.publish(PlayerOpenContainerAfterEvent(
                            *event.player(),
                            *packet.mPos,
                            packet.mContainerId,
                            packet.mType,
                            *packet.mEntityUniqueID
                        ));
                    }
                }
            )
        );
    });
}

} // namespace ila::mc::inline world::inline actor::inline player