#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Event.h>
#include <mc/network/NetworkIdentifier.h>
#include <mc/network/NetworkSystem.h>
#include <mc/network/Packet.h>
#include <mc/world/actor/player/Player.h>

namespace ila::mc::inline packet {

class PacketEvent : public ll::event::Event {
private:
    NetworkSystem&           mNetworkSystem;
    NetworkIdentifier const& mNetworkIdentifier;
    SubClientId              mSenderSubId;
    Packet&                  mPacket;

public:
    constexpr explicit PacketEvent(
        NetworkSystem&           networkSystem,
        NetworkIdentifier const& networkIdentifier,
        SubClientId              senderSubId,
        Packet&                  packet
    )
    : mNetworkSystem(networkSystem),
      mNetworkIdentifier(networkIdentifier),
      mSenderSubId(senderSubId),
      mPacket(packet) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;

public:
    NetworkSystem&           networkSystem() const { return mNetworkSystem; }
    NetworkIdentifier const& networkIdentifier() const { return mNetworkIdentifier; }
    SubClientId              senderSubId() const { return mSenderSubId; }
    Packet&                  packet() const { return mPacket; }

    /**
     *  @brief On the client side, this returns the LocalPlayer;
     *  @brief On the server side, it returns the ServerPlayer of the sending client.;
     *  @warning It returns nullptr if the player object is not initialized.
     */
    virtual optional_ref<Player> player() const;
    /**
     *  @brief Returns true if the event is on the server side;
     */
    bool isServerSide() const { return networkSystem().isServer(); }
};

} // namespace ila::mc::inline packet