#include "ila/event/packet/BroadcastPacketEvent.h"
#include "ila/base/Gloabl.i.h"
#include <iterator>
#include <ll/api/base/Containers.h>
#include <ll/api/event/Event.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/service/Bedrock.h>
#include <magic_enum.hpp>
#include <mc/deps/core/utility/optional_ref.h>
#include <mc/deps/nbt/CompoundTag.h>
#include <mc/deps/nbt/ListTag.h>
#include <mc/network/BatchedNetworkPeer.h>
#include <mc/network/Compressibility.h>
#include <mc/network/NetworkConnection.h>
#include <mc/network/NetworkIdentifierWithSubId.h>
#include <mc/network/NetworkPeer.h>
#include <mc/network/NetworkSystem.h>
#include <mc/network/Packet.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/server/ServerPlayer.h>
#include <mc/world/actor/player/Player.h>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace ila::packet {

void BroadcastPacketEvent::serialize(CompoundTag& nbt) const {
    Event::serialize(nbt);
    nbt["network_system"] = serializeRefObj(mNetworkSystem);
    // clang-format off
    nbt["targets"] = mTargets
        | std::views::transform([](NetworkIdentifierWithSubId const& id) {
            return CompoundTag{
                { "network_identifier", serializeRefObj(id.id)                  },
                { "sender_sub_id",      magic_enum::enum_name(id.subClientId) }
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

    auto event = eventPromise(BroadcastingPacketEvent{*this, idsSet, pkt}).publish();
    if (event || idsSet.empty()) return;

    gBroadcastPacketInfo.emplace(ll::DenseSet<NetworkPeer*>{});
    origin(std::vector<NetworkIdentifierWithSubId>{event->targets().begin(), event->targets().end()}, packet);
    for (auto it = idsSet.begin(); it != idsSet.end();) {
        if (auto* connect = getConnectionFromId(it->id);
            !connect || !gBroadcastPacketInfo->contains(connect->mPeer.get())) {
            it = idsSet.erase(it);
        } else {
            ++it;
        }
    }
    if (!idsSet.empty()) eventPromise(BroadcastedPacketEvent{*this, idsSet, pkt}).publish();
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

} // namespace ila::packet