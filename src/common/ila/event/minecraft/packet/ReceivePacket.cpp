#include "ila/event/minecraft/packet/ReceivePacket.i.h"
#include <ll/api/service/Bedrock.h>
#include <mc/client/network/ClientNetworkHandler.h>
#include <mc/deps/core/debug/BedrockLog.h>
#include <mc/network/IPacketHandlerDispatcher.h>
#include <mc/network/MinecraftPackets.h>
#include <mc/network/NetworkConnection.h>
#include <mc/network/NetworkIdentifierWithSubId.h>

namespace ila::mc::inline packet {

IncomingPacketFilterResult handleReceive(
    NetEventCallback&                 self,
    NetworkIdentifierWithSubId const& id,
    bool                              isServerSide
) {
    auto networkSystem = ll::service::getNetworkSystem(!isServerSide);
    if (!networkSystem) return IncomingPacketFilterResult::Allowed;

    auto* connect = networkSystem->getConnectionFromId(id.id);
    if (!connect || connect->mShouldCloseConnection) return IncomingPacketFilterResult::Allowed;

    ReadOnlyBinaryStream stream{networkSystem->mReceiveBuffer.get(), false};
    auto                 header = stream.getUnsignedVarInt();
    if (!header) {
        va_list args{};
        BedrockLog::log_va(
            BedrockLog::LogCategory::LogArea,
            {1},
            BedrockLog::LogRule::DefaultRules,
            LogAreaID::LogAreaNetwork,
            static_cast<uint>(Bedrock::LogLevel::Error().mType),
            __FUNCTION__,
            __LINE__,
            header.error().mError.message().c_str(),
            args
        );
    }

    auto packet = MinecraftPackets::createPacket(static_cast<MinecraftPacketIds>(header.value() & 0x3ff));
    if (!packet) return IncomingPacketFilterResult::Allowed;

    if (auto result = packet->checkSize(stream.mView.size() - stream.mReadPointer, true); !result) {
        return IncomingPacketFilterResult::Allowed;
    }
    if (auto result = packet->read(stream); !result) return IncomingPacketFilterResult::Allowed;
    if (!packet->mHandler) return IncomingPacketFilterResult::Allowed;

    auto now                  = std::chrono::steady_clock::now();
    connect->mLastPacketTime  = now;
    packet->mReceiveTimepoint = now;

    ReceivingPacketEvent event{*networkSystem, id.id, id.subClientId, *packet};
    getLLEventBus().publish(event);
    if (event.isCancelled()) return IncomingPacketFilterResult::RejectedSilently;
    packet->mHandler->handle(id.id, self, packet);
    getLLEventBus().publish(ReceivedPacketEvent{*networkSystem, id.id, id.subClientId, *packet});

    return IncomingPacketFilterResult::RejectedSilently;
}

} // namespace ila::mc::inline packet