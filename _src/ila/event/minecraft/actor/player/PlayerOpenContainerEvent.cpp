#include "ila/event/minecraft/actor/player/PlayerOpenContainerEvent.h"
#include "ila/base/Gloabl.h"
#include "ila/event/minecraft/server/SendPacketEvent.h"
#include <ll/api/base/StdInt.h>
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/player/ServerPlayerEvent.h>
#include <magic_enum.hpp>
#include <mc/deps/shared_types/legacy/ContainerType.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/nbt/ByteTag.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/nbt/StringTag.h>
#include <mc/network/NetworkBlockPosition.h>
#include <mc/network/packet/ContainerOpenPacket.h>
#include <mc/world/ContainerID.h>
#include <mc/world/level/BlockPos.h>

namespace ila::mc::inline actor::inline player
{

void PlayerOpenContainerBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["containerBlockPos"] = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["containerId"]       = static_cast<schar>(mContainerId);
    nbt["containerType"]     = magic_enum::enum_name(mContainerType);
    nbt["containerActorId"]  = mContainerActorId.rawID;
}
void PlayerOpenContainerBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mPos.x       = nbt["containerBlockPos"][0];
    mPos.y       = nbt["containerBlockPos"][1];
    mPos.z       = nbt["containerBlockPos"][2];
    mContainerId = static_cast<ContainerID>(nbt["containerId"].get<ByteTag>().data);
    mContainerType =
        magic_enum::enum_cast<SharedTypes::Legacy::ContainerType>(nbt["containerType"].get<StringTag>())
            .value_or(mContainerType);
    mContainerActorId.rawID = nbt["containerActorId"];
}

void PlayerOpenContainerAfterEvent::serialize(CompoundTag& nbt) const
{
    ServerPlayerEvent::serialize(nbt);
    nbt["containerBlockPos"] = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["containerId"]       = static_cast<schar>(mContainerId);
    nbt["containerType"]     = magic_enum::enum_name(mContainerType);
    nbt["containerActorId"]  = mContainerActorId.rawID;
}

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
                            *player,
                            *packet.mPos,
                            packet.mContainerId,
                            packet.mType,
                            *packet.mEntityUniqueID
                        );
                        LLEventBus.publish(beforeEvent);
                        if (beforeEvent.isCancelled())
                        {
                            event.cancel();
                            player->doDeleteContainerManager(false);
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
                            *player,
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

} // namespace ila::mc::inline actor::inline player
