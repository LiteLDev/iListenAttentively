#include "ila/event/minecraft/server/ReceivePacketEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/service/Bedrock.h>
#include <mc/network/BatchedNetworkPeer.h>
#include <mc/network/MinecraftPackets.h>
#include <mc/network/ServerNetworkHandler.h>

namespace ila::mc::inline server
{

void ReceivePacketEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["packet"]            = serializeRefObj(packet());
    nbt["networkIdentifier"] = serializeRefObj(networkIdentifier());
}
Packet&                    ReceivePacketEvent::packet() const { return mPacket; }
NetworkIdentifier&         ReceivePacketEvent::networkIdentifier() const { return mNetworkIdentifier; }
optional_ref<ServerPlayer> ReceivePacketEvent::player() const
{
    return ll::service::getServerNetworkHandler()->_getServerPlayer(
        networkIdentifier(),
        SubClientId::PrimaryClient
    );
}

LL_TYPE_INSTANCE_HOOK(
    ReceivePacketEventHook,
    HookPriority::Normal,
    BatchedNetworkPeer,
    &BatchedNetworkPeer::$receivePacket,
    NetworkPeer::DataStatus,
    std::string&                                                  pOutData,
    std::shared_ptr<std::chrono::steady_clock::time_point> const& pTimepointPtr
)
{
    std::string data;
    if (auto result = origin(data, pTimepointPtr); result != NetworkPeer::DataStatus::HasData)
    {
        pOutData = std::move(data);
        return result;
    }

    ReadOnlyBinaryStream input { data, true };
    auto pkt = MinecraftPackets::createPacket(static_cast<MinecraftPacketIds>(*input.getUnsignedVarInt()));
    pkt->read(input);

    auto beforeEvent = ReceivePacketEvent(*pkt, getNetworkIdentifier(*this));
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return NetworkPeer::DataStatus::NoData; }

    BinaryStream output;
    output.writeUnsignedVarInt(static_cast<uint>(pkt->getId()), nullptr, nullptr);
    pkt->write(output);
    pOutData = std::move(output.mBuffer);

    return NetworkPeer::DataStatus::HasData;
}

Event_Hook_Factory_Base(ReceivePacket, <ReceivePacketEventHook>);

} // namespace ila::mc::inline server