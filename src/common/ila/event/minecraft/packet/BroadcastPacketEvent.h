#pragma once
#include "ila/base/Macro.h"
#include "patch_mc/network/NetworkIdentifierWithSubId.h"
#include <ll/api/base/Containers.h>
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/Event.h>
#include <mc/network/NetworkSystem.h>
#include <mc/network/Packet.h>

namespace ila::mc::inline packet {

class BroadcastPacketEvent : public ll::event::Event {
private:
    NetworkSystem&                            mNetworkSystem;
    ll::DenseSet<NetworkIdentifierWithSubId>& mTargets;
    Packet&                                   mPacket;

public:
    explicit BroadcastPacketEvent(
        NetworkSystem&                            networkSystem,
        ll::DenseSet<NetworkIdentifierWithSubId>& targets,
        Packet&                                   packet
    )
    : mNetworkSystem(networkSystem),
      mTargets(targets),
      mPacket(packet) {}

    ILAPI void serialize(CompoundTag& nbt) const override;

public:
    NetworkSystem&                            networkSystem() const { return mNetworkSystem; }
    ll::DenseSet<NetworkIdentifierWithSubId>& targets() const { return mTargets; }
    Packet&                                   packet() const { return mPacket; }

    virtual optional_ref<Player> player(NetworkIdentifierWithSubId const& id) const;
};

/** @warning This event is not available on the client side. */
class BroadcastingPacketEvent final : public ll::event::Cancellable<BroadcastPacketEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class BroadcastedPacketEvent final : public BroadcastPacketEvent {
public:
    using BroadcastPacketEvent::BroadcastPacketEvent;
};

} // namespace ila::mc::inline packet