#include "ila/event/minecraft/packet/BroadcastPacketEvent.h"
#include "ila/base/Gloabl.i.h"
#include <ll/api/service/Bedrock.h>
#include <mc/network/BatchedNetworkPeer.h>
#include <mc/network/NetworkConnection.h>
#include <mc/network/NetworkIdentifierWithSubId.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/server/ServerPlayer.h>
#include <optional>
#include <ranges>

namespace ila::mc::inline packet {

void BroadcastPacketEvent::serialize(CompoundTag& nbt) const {
    Event::serialize(nbt);
    nbt["network_system"] = serializeRefObj(mNetworkSystem);
    // clang-format off
    nbt["targets"] = mTargets
        | std::views::transform([](NetworkIdentifierWithSubId const& id) {
            return CompoundTag{
                { "network_identifier", serializeRefObj(id.id)                                              },
                { "sender_sub_id",      ll::reflection::serialize<CompoundTagVariant>(id.subClientId).value() }
            };
        })
        | std::ranges::to<ListTag>();
    // clang-format on
    nbt["packet"] = serializeRefObj(mPacket);
}

optional_ref<Player> BroadcastPacketEvent::player(NetworkIdentifierWithSubId const& id) const {
    if (!mNetworkSystem.isServer()) return std::nullopt;
    return ll::service::getServerNetworkHandler()->_getServerPlayer(id.id, id.subClientId);
}

thread_local std::optional<ll::DenseSet<NetworkPeer*>> gBroadcastPacketInfo{std::nullopt};

LL_TYPE_INSTANCE_HOOK(
    BroadcastPacketEventHook1,
    HookPriority::Normal,
    NetworkSystem,
    &NetworkSystem::sendToMultiple,
    void,
    std::vector<NetworkIdentifierWithSubId> const& ids,
    Packet const&                                  packet
) {
    if (ids.empty()) return;

    auto&                                    pkt = const_cast<Packet&>(packet);
    ll::DenseSet<NetworkIdentifierWithSubId> idsSet(
        std::make_move_iterator(ids.begin()),
        std::make_move_iterator(ids.end())
    );

    BroadcastingPacketEvent event{*this, idsSet, pkt};
    getLLEventBus().publish(event);
    if (event.isCancelled() || idsSet.empty()) return;

    gBroadcastPacketInfo.emplace(ll::DenseSet<NetworkPeer*>{});
    origin(std::vector<NetworkIdentifierWithSubId>{event.targets().begin(), event.targets().end()}, packet);
    for (auto it = idsSet.begin(); it != idsSet.end();) {
        if (auto* connect = getConnectionFromId(it->id);
            !connect || !gBroadcastPacketInfo->contains(connect->mPeer.get())) {
            it = idsSet.erase(it);
        } else {
            ++it;
        }
    }
    if (!idsSet.empty()) getLLEventBus().publish(BroadcastedPacketEvent{*this, idsSet, pkt});
    gBroadcastPacketInfo.reset();
}

LL_TYPE_INSTANCE_HOOK(
    BroadcastPacketEventHook2,
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
    if (!gBroadcastPacketInfo || length == mOutgoingData->mOwnedBuffer.size()) return;
    gBroadcastPacketInfo->insert(this);
}

EventHook(BroadcastingPacketEvent, BroadcastedPacketEvent, <BroadcastPacketEventHook1, BroadcastPacketEventHook2>);

} // namespace ila::mc::inline packet