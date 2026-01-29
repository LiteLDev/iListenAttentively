#include "ila/event/minecraft/actor/player/PlayerCloseContainerEvent.h"
#include "ila/base/Gloabl.h"
#include "ila/event/minecraft/server/SendPacketEvent.h"
#include <ll/api/base/StdInt.h>
#include <ll/api/event/player/ServerPlayerEvent.h>
#include <magic_enum.hpp>
#include <mc/deps/shared_types/legacy/ContainerType.h>
#include <mc/nbt/ByteTag.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/StringTag.h>
#include <mc/network/packet/ContainerClosePacket.h>
#include <mc/world/ContainerID.h>

namespace ila::mc::inline actor::inline player
{

void PlayerCloseContainerBeforeEvent::serialize(CompoundTag& nbt) const
{
    ServerPlayerEvent::serialize(nbt);
    nbt["containerId"]          = static_cast<schar>(mContainerId);
    nbt["containerType"]        = magic_enum::enum_name(mContainerType);
    nbt["serverInitiatedClose"] = mServerInitiatedClose;
}
void PlayerCloseContainerBeforeEvent::deserialize(CompoundTag const& nbt)
{
    ServerPlayerEvent::deserialize(nbt);
    mContainerId = static_cast<ContainerID>(nbt["containerId"].get<ByteTag>().data);
    mContainerType =
        magic_enum::enum_cast<SharedTypes::Legacy::ContainerType>(nbt["containerType"].get<StringTag>())
            .value_or(mContainerType);
    mServerInitiatedClose = nbt["serverInitiatedClose"];
}

void PlayerCloseContainerAfterEvent::serialize(CompoundTag& nbt) const
{
    ServerPlayerEvent::serialize(nbt);
    nbt["containerId"]          = static_cast<schar>(mContainerId);
    nbt["containerType"]        = magic_enum::enum_name(mContainerType);
    nbt["serverInitiatedClose"] = mServerInitiatedClose;
}

Event_Listener_Factory(PlayerCloseContainerBefore)
{
    nextTick([this]() -> void { // Prevent deadlock
        mListeners.emplace_back(
            LLEventBus.emplaceListener<ila::mc::server::SendPacketBeforeEvent<ContainerClosePacket>>(
                [](ila::mc::server::SendPacketBeforeEvent<ContainerClosePacket>& event) -> void {
                    if (auto player = event.player(); player)
                    {
                        auto& packet = event.packet();
                        LLEventBus.publish(PlayerCloseContainerBeforeEvent(
                            *player,
                            packet.mContainerId,
                            packet.mContainerType,
                            packet.mServerInitiatedClose
                        ));
                    }
                }
            )
        );
    });
}

Event_Listener_Factory(PlayerCloseContainerAfter)
{
    nextTick([this]() -> void { // Prevent deadlock
        mListeners.emplace_back(
            LLEventBus.emplaceListener<ila::mc::server::SendPacketAfterEvent<ContainerClosePacket>>(
                [](ila::mc::server::SendPacketAfterEvent<ContainerClosePacket>& event) -> void {
                    if (auto player = event.player(); player)
                    {
                        auto& packet = event.packet();
                        LLEventBus.publish(PlayerCloseContainerAfterEvent(
                            *player,
                            packet.mContainerId,
                            packet.mContainerType,
                            packet.mServerInitiatedClose
                        ));
                    }
                }
            )
        );
    });
}

} // namespace ila::mc::inline actor::inline player
