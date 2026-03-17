#include "ila/event/packet/ReceivePacketEvent.h"
#include "ila/base/Gloabl.i.h"
#include "ila/event/packet/ReceivePacketEvent.i.h"
#include <ll/api/base/StdInt.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/service/Bedrock.h>
#include <mc/network/IncomingPacketFilterResult.h>
#include <mc/network/MinecraftPacketIds.h>
#include <mc/network/NetEventCallback.h>
#include <mc/network/NetworkIdentifierWithSubId.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/server/ServerPlayer.h>

namespace ila::packet {

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

} // namespace ila::packet