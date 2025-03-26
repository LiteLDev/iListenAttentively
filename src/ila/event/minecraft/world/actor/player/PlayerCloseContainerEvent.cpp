#include "ila/event/minecraft/world/actor/player/PlayerCloseContainerEvent.h"
#include "ila/base/Gloabl.h"
#include "ila/event/minecraft/server/SendPacketEvent.h"
#include <mc/network/packet/ContainerClosePacket.h>

namespace ila::mc::inline world::inline actor::inline player
{

void PlayerCloseContainerBeforeEvent::serialize(CompoundTag& nbt) const
{
    ServerPlayerEvent::serialize(nbt);
    nbt["containerId"]          = static_cast<schar>(containerId());
    nbt["containerType"]        = magic_enum::enum_name(containerType());
    nbt["serverInitiatedClose"] = serverInitiatedClose();
}
void PlayerCloseContainerBeforeEvent::deserialize(CompoundTag const& nbt)
{
    ServerPlayerEvent::deserialize(nbt);
    containerId() = static_cast<ContainerID>(nbt["containerId"].get<ByteTag>().data);
    containerType() =
        magic_enum::enum_cast<SharedTypes::Legacy::ContainerType>(nbt["containerType"].get<StringTag>())
            .value_or(containerType());
    serverInitiatedClose() = nbt["serverInitiatedClose"];
}
ContainerID& PlayerCloseContainerBeforeEvent::containerId() const { return mContainerId; }
SharedTypes::Legacy::ContainerType& PlayerCloseContainerBeforeEvent::containerType() const
{
    return mContainerType;
}
bool& PlayerCloseContainerBeforeEvent::serverInitiatedClose() const { return mServerInitiatedClose; }

void PlayerCloseContainerAfterEvent::serialize(CompoundTag& nbt) const
{
    ServerPlayerEvent::serialize(nbt);
    nbt["containerId"]          = static_cast<schar>(containerId());
    nbt["containerType"]        = magic_enum::enum_name(containerType());
    nbt["serverInitiatedClose"] = serverInitiatedClose();
}
ContainerID const& PlayerCloseContainerAfterEvent::containerId() const { return mContainerId; }
SharedTypes::Legacy::ContainerType const& PlayerCloseContainerAfterEvent::containerType() const
{
    return mContainerType;
}
bool const& PlayerCloseContainerAfterEvent::serverInitiatedClose() const { return mServerInitiatedClose; }

Event_Listener_Factory(PlayerCloseContainerBefore)
{
    nextTick([this]() -> void { // Prevent deadlock
        mListeners.emplace_back(LLEventBus.emplaceListener<ila::mc::server::SendPacketBeforeEvent>(
            [](ila::mc::server::SendPacketBeforeEvent& event) -> void {
                if (event.packet().getId() != MinecraftPacketIds::ContainerClose || event.isBroadcast()
                    || !event.player().has_value())
                {
                    return;
                }
                auto& packet = static_cast<ContainerClosePacket&>(event.packet());
                LLEventBus.publish(PlayerCloseContainerBeforeEvent(
                    *event.player(),
                    packet.mContainerId,
                    packet.mContainerType,
                    packet.mServerInitiatedClose
                ));
            }
        ));
    });
}

Event_Listener_Factory(PlayerCloseContainerAfter)
{
    nextTick([this]() -> void { // Prevent deadlock
        mListeners.emplace_back(LLEventBus.emplaceListener<ila::mc::server::SendPacketAfterEvent>(
            [](ila::mc::server::SendPacketAfterEvent& event) -> void {
                if (event.packet().getId() != MinecraftPacketIds::ContainerClose || event.isBroadcast()
                    || !event.player().has_value())
                {
                    return;
                }
                auto& packet = static_cast<ContainerClosePacket const&>(event.packet());
                LLEventBus.publish(PlayerCloseContainerAfterEvent(
                    *event.player(),
                    packet.mContainerId,
                    packet.mContainerType,
                    packet.mServerInitiatedClose
                ));
            }
        ));
    });
}

} // namespace ila::mc::inline world::inline actor::inline player