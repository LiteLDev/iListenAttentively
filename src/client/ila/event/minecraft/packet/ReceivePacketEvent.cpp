#include "ila/event/minecraft/packet/ReceivePacketEvent.i.h"
#include <mc/client/network/ClientNetworkHandler.h>
#include <mc/client/player/LocalPlayer.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/server/ServerPlayer.h>

namespace ila::mc::inline packet {

LL_TYPE_INSTANCE_HOOK(
    ReceivePacketEventHook1,
    HookPriority::Low,
    ServerNetworkHandler,
    &ServerNetworkHandler::$allowIncomingPacketId,
    IncomingPacketFilterResult,
    NetworkIdentifierWithSubId const& id,
    MinecraftPacketIds                packetId,
    uint64                            packetSize
) {
    if (auto result = origin(id, packetId, packetSize); result != IncomingPacketFilterResult::Allowed) {
        return result;
    }
    return handleReceive(*thisFor<NetEventCallback>(), id, true);
}

LL_TYPE_INSTANCE_HOOK(
    ReceivePacketEventHook2,
    HookPriority::Low,
    ClientNetworkHandler,
    &ClientNetworkHandler::$allowIncomingPacketId,
    IncomingPacketFilterResult,
    NetworkIdentifierWithSubId const& id,
    MinecraftPacketIds                packetId,
    uint64                            packetSize
) {
    if (auto result = origin(id, packetId, packetSize); result != IncomingPacketFilterResult::Allowed) {
        return result;
    }
    return handleReceive(*thisFor<NetEventCallback>(), id, false);
}

EventHook(ReceivingPacketEvent, ReceivedPacketEvent, <ReceivePacketEventHook1, ReceivePacketEventHook2>);

} // namespace ila::mc::inline packet