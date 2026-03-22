// clang-format off
#pragma include_alias("mc/deps/raknet/RNS2RecvStruct.h", "patch_mc/deps/raknet/RNS2RecvStruct.h")
#pragma include_alias(<mc/deps/raknet/RNS2RecvStruct.h>, <patch_mc/deps/raknet/RNS2RecvStruct.h>)
#pragma include_alias("mc/deps/raknet/BitStream.h", "patch_mc/deps/raknet/BitStream.h")
#pragma include_alias(<mc/deps/raknet/BitStream.h>, <patch_mc/deps/raknet/BitStream.h>)
// clang-format on
#include "ila/event/server/ServerPongEvent.h"
#include "ila/base/Gloabl.i.h"
#include "ila/utils/StringUtils.i.h"
#include "ll/api/base/Containers.h"
#include "ll/api/reflection/Reflection.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/utils/StringUtils.h"
#include <array>
#include <boost/pfr/core.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <initializer_list>
#include <ll/api/base/StdInt.h>
#include <ll/api/event/Event.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>
#include <magic_enum.hpp>
#include <mc/deps/raknet/DefaultMessageIDTypes.h>
#include <mc/deps/raknet/RNS2RecvStruct.h>
#include <mc/deps/raknet/RNS2_SendParameters.h>
#include <mc/deps/raknet/RakNet.h>
#include <mc/deps/raknet/RakNetSocket2.h>
#include <mc/deps/raknet/RakPeer.h>
#include <mc/deps/raknet/SystemAddress.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/CompoundTagVariant.h>
#include <patch_mc/deps/raknet/BitStream.h>
#include <ranges>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <winsock2.h>
#include <ws2def.h>

namespace ila::server {

void ServerPongEvent::serialize(CompoundTag& nbt) const {
    Event::serialize(nbt);
    nbt["address"] = CompoundTag{
        {"ip",   mAddress.first },
        {"port", mAddress.second}
    };
    auto pong = reflection::serialize<CompoundTagVariant>(mPongData).value();
    for (auto& [key, value] : pong.get<CompoundTag>()) {
        nbt[string_utils::camelToSnakeWithoutM(key)] = std::move(value);
    }

    // JS Port Note: value < 0 ? value + (1 << 16) : value
}

void ServerPongEvent::deserialize(CompoundTag const& nbt) {
    Event::deserialize(nbt);
    ll::reflection::forEachMember(mPongData, [&nbt](std::string_view name, auto&& member) {
        auto key = string_utils::camelToSnakeWithoutM(name);
        if (nbt.contains(key)) {
            reflection::deserialize(member, nbt[key]).value();
        }
    });
}

LL_TYPE_INSTANCE_HOOK(
    ServerPongEventHook,
    HookPriority::Normal,
    RakNet::RakPeer,
    &RakNet::RakPeer::$OnRNS2Recv,
    void,
    RakNet::RNS2RecvStruct* recvStruct
) {
    // clang-format off
    constexpr static std::array<uint8, 16> vscOfflineMessageDataId = {
        0x00, 0xFF, 0xFF, 0x00,
        0xFE, 0xFE, 0xFE, 0xFE,
        0xFD, 0xFD, 0xFD, 0xFD,
        0x12, 0x34, 0x56, 0x78
    };
    // clang-format on
    auto& rakPeer = *ll::service::getRakPeer();

    if (static_cast<DefaultMessageIDTypes>(recvStruct->mData[0]) != DefaultMessageIDTypes::UnconnectedPing) {
        return origin(recvStruct);
    }

    if (static_cast<size_t>(recvStruct->mBytesRead)
        < sizeof(uint8) + sizeof(uint64) + sizeof(vscOfflineMessageDataId)) {
        return origin(recvStruct);
    }

    char* pongData{nullptr};
    uint  pongLength{0};
    rakPeer.GetOfflinePingResponse(&pongData, &pongLength);
    if (pongLength < 2 || (pongData[0] << 8 | pongData[1]) != static_cast<int>(pongLength) - 2) {
        return origin(recvStruct);
    }

    auto address = [](RakNet::SystemAddress& address) {
        char buffer[56]{};
        address.ToString(false, buffer, '|');
        return std::pair<std::string, ushort>{buffer, ntohs(address.address.addr4.as<sockaddr_in>().sin_port)};
    }(recvStruct->mSystemAddress);

    auto pong = [](std::string_view pongData) {
        // clang-format off
        ServerPongEvent::PongData result{};
        auto parts = pongData | std::views::split(';') | std::views::drop(1) | std::ranges::to<std::vector<std::string>>();
        if (parts.size() > 7) parts.erase(parts.begin() + 7);
        boost::pfr::for_each_field(result, [&](auto&& field, size_t index) {
            if (index >= parts.size()) return;
            ll::Overloaded{
                [&](std::string& value) {
                    value = parts[index];
                },
                [&]<typename T> requires std::is_enum_v<T> (T& value){
                    value = magic_enum::enum_cast<T>(parts[index]).value_or(T{});
                },
                [&]<typename T> requires std::is_integral_v<T> (T& value) {
                    value = ll::string_utils::svtonum<T>(parts[index], nullptr, 10).value_or(0);
                },
                [&](auto&&) {} // std::vector<std::string> mOther
            }(field);
        });
        if (parts.size() > 10) result.mOthers.assign(parts.begin() + 11, parts.end() - 1);
        return result;
        // clang-format on
    }({pongData + 2, pongLength - 2});

    if (eventPromise(SendingServerPongEvent(address, pong)).publish()) return;

    RakNet::BitStream is(reinterpret_cast<uchar*>(recvStruct->mData.data()), recvStruct->mBytesRead, false);
    is.IgnoreBits(8);
    auto sendPingTime = is.Read<uint64>();
    is.IgnoreBytes(sizeof(vscOfflineMessageDataId));
    is.Read<uint64>();

    auto response = fmt::format(
        "MCPE;{};{};{};{};{};{};{};{};1;{};{};0;{}",
        pong.mMotd,
        pong.mNetworkProtocolVersion,
        pong.mMinecraftVersionNetwork,
        pong.mNumPlayers,
        pong.mMaxPlayers,
        pong.mServerGuid,
        pong.mLevelName,
        magic_enum::enum_name(pong.mGameMode),
        pong.mLocalPort,
        pong.mLocalPortV6,
        pong.mOthers.empty() ? "" : fmt::format("{};", fmt::join(pong.mOthers, ";"))
    );
    RakNet::BitStream os;
    os.Write<uint8>(static_cast<uint8>(DefaultMessageIDTypes::UnconnectedPong));
    os.Write<uint64>(sendPingTime.value_or(0));
    os.Write<uint64>(rakPeer.GetMyGUID().g);
    os.WriteAlignedBytes(vscOfflineMessageDataId.data(), sizeof(vscOfflineMessageDataId));
    os.Write<uint16>(static_cast<uint16>(response.size()));
    os.Write(response.data(), static_cast<uint>(response.size()));

    RakNet::RNS2_SendParameters bsp;
    bsp.data          = reinterpret_cast<char*>(os.mData);
    bsp.length        = static_cast<int>((os.mNumberOfBitsUsed + 7) >> 3);
    bsp.systemAddress = recvStruct->mSystemAddress;
    auto location     = std::source_location::current();
    recvStruct->mSocket->Send(&bsp, location.file_name(), location.line());

    eventPromise(SentServerPongEvent(address, pong)).publish();
}

EventHook(SendingServerPongEvent, SentServerPongEvent, <ServerPongEventHook>);

} // namespace ila::server