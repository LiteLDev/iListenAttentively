#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <mc/deps/core/utility/optional_ref.h>
#include <mc/network/LoopbackPacketSender.h>
#include <mc/network/packet/Packet.h>
#include <mc/server/ServerPlayer.h>

namespace ila::mc::inline server
{
class ReceivePacketEvent final : public ll::event::Cancellable<ll::event::Event>
{
protected:
    Packet&            mPacket;
    NetworkIdentifier& mNetworkIdentifier;

public:
    constexpr explicit ReceivePacketEvent(Packet& packet, NetworkIdentifier& networkIdentifier)
        : mPacket(packet)
        , mNetworkIdentifier(networkIdentifier)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

    ILNDAPI Packet&            packet() const;
    ILNDAPI NetworkIdentifier& networkIdentifier() const;
    ILNDAPI optional_ref<ServerPlayer> player() const;
}; // class ReceivePacketEvent
} // namespace ila::mc::inline server