#include "ila/base/Gloabl.i.h"
#include "ila/event/minecraft/packet/ReceivePacket.h"
#include "ila/event/minecraft/packet/ReceivePacket.i.h"
#include <ll/api/service/Bedrock.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/server/ServerPlayer.h>

namespace ila::mc::inline packet {

LL_TYPE_INSTANCE_HOOK(
    ReceivePacketEventHook,
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

EventHook(ReceivingPacketEvent, ReceivedPacketEvent, <ReceivePacketEventHook>);

} // namespace ila::mc::inline packet