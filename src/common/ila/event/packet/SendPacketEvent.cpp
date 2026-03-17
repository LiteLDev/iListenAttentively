#include "ila/event/packet/SendPacketEvent.h"
#include "ila/base/Gloabl.i.h"
#include <ll/api/memory/Hook.h>
#include <mc/common/SubClientId.h>
#include <mc/network/BatchedNetworkPeer.h>
#include <mc/network/Compressibility.h>
#include <mc/network/NetworkConnection.h>
#include <mc/network/NetworkIdentifier.h>
#include <mc/network/NetworkIdentifierWithSubId.h>
#include <mc/network/NetworkPeer.h>
#include <mc/network/NetworkSystem.h>
#include <mc/network/Packet.h>
#include <optional>
#include <string>
#include <tuple>

namespace ila::packet {

thread_local std::optional<std::tuple<NetworkSystem*, NetworkIdentifier, SubClientId, Packet*>> gSendPacketInfo{
    std::nullopt
};

LL_TYPE_INSTANCE_HOOK(
    SendPacketEventHook1,
    HookPriority::Normal,
    NetworkSystem,
    &NetworkSystem::send,
    void,
    NetworkIdentifier const& id,
    Packet const&            packet,
    SubClientId              recipientSubId
) {
    auto&              pkt = const_cast<Packet&>(packet);
    SendingPacketEvent event{*this, id, recipientSubId, pkt};
    getLLEventBus().publish(event);
    if (event.isCancelled()) return;
    gSendPacketInfo.emplace(std::tuple{this, id, recipientSubId, &pkt});
    origin(id, pkt, recipientSubId);
    gSendPacketInfo.reset();
}

LL_TYPE_INSTANCE_HOOK(
    SendPacketEventHook2,
    HookPriority::Normal,
    BatchedNetworkPeer,
    &BatchedNetworkPeer::$sendPacket,
    void,
    std::string const&       data,
    NetworkPeer::Reliability reliability,
    Compressibility          compressible
) {
    auto length = mOutgoingData->mOwnedBuffer.size();
    origin(data, reliability, compressible);
    if (!gSendPacketInfo || length == mOutgoingData->mOwnedBuffer.size()) return;
    auto& [net, id, subId, pkt] = *gSendPacketInfo;
    if (net->getConnectionFromId(id)->mPeer.get() != this) return;
    getLLEventBus().publish(SentPacketEvent{*net, id, subId, *pkt});
}

EventHook(SendingPacketEvent, SentPacketEvent, <SendPacketEventHook1, SendPacketEventHook2>);

} // namespace ila::packet