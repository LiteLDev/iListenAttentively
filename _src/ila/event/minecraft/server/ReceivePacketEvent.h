#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <mc/deps/core/utility/optional_ref.h>

// clang-format off
class NetworkIdentifier;
class ServerPlayer;
class Packet;
// clang-format on

namespace ila::mc::inline server
{

class IReceivePacketBeforeEvent : public ll::event::Cancellable<ll::event::Event>
{
public:
    Packet&                  mPacket;
    NetworkIdentifier const& mNetworkIdentifier;

public:
    constexpr explicit IReceivePacketBeforeEvent(Packet& packet, NetworkIdentifier const& networkIdentifier)
        : mPacket(packet)
        , mNetworkIdentifier(networkIdentifier)
    {
    }

    ILAPI void                 serialize(CompoundTag& nbt) const override;
    optional_ref<ServerPlayer> player() const;

}; // class ReceivePacketEvent

template<std::derived_from<Packet> PacketType = Packet>
class ReceivePacketBeforeEvent final : public IReceivePacketBeforeEvent
{
public:
    constexpr explicit ReceivePacketBeforeEvent(
        PacketType&              packet,
        NetworkIdentifier const& networkIdentifier
    )
        : IReceivePacketBeforeEvent(packet, networkIdentifier)
    {
    }

    PacketType& packet() const { return static_cast<PacketType&>(mPacket); }
};

class IReceivePacketAfterEvent : public ll::event::Cancellable<ll::event::Event>
{
public:
    Packet const&            mPacket;
    NetworkIdentifier const& mNetworkIdentifier;

public:
    constexpr explicit IReceivePacketAfterEvent(
        Packet const&            packet,
        NetworkIdentifier const& networkIdentifier
    )
        : mPacket(packet)
        , mNetworkIdentifier(networkIdentifier)
    {
    }

    ILAPI void                 serialize(CompoundTag& nbt) const override;
    optional_ref<ServerPlayer> player() const;

}; // class ReceivePacketEvent

template<std::derived_from<Packet> PacketType = Packet>
class ReceivePacketAfterEvent final : public IReceivePacketAfterEvent
{
public:
    constexpr explicit ReceivePacketAfterEvent(
        PacketType const&        packet,
        NetworkIdentifier const& networkIdentifier
    )
        : IReceivePacketAfterEvent(packet, networkIdentifier)
    {
    }

    PacketType const& packet() const { return static_cast<PacketType const&>(mPacket); }
};

} // namespace ila::mc::inline server
