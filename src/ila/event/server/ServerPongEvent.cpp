#include "ila/event/server/ServerPongEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/Versions.h>
#include <ll/api/service/TargetedBedrock.h>
#include <mc/deps/core/utility/BinaryStream.h>
#include <mc/deps/core/utility/ReadOnlyBinaryStream.h>
#include <mc/deps/nether_net/NetworkID.h>
#include <mc/deps/nether_net/lan/LanThreadManager.h>
#include <mc/deps/nether_net/signaling/http/HttpConnection.h>
#include <mc/deps/nether_net/signaling/http/HttpRequest.h>
#include <mc/deps/nether_net/signaling/http/HttpResponse.h>
#include <mc/deps/nether_net/signaling/http/HttpSignalingServer.h>
#include <mc/deps/raknet/RNS2_SendParameters.h>
#include <mc/deps/raknet/RNS2_Windows.h>
#include <mc/deps/raknet/SystemAddress.h>
#include <mc/external/webrtc/Socket.h>
#include <mc/external/webrtc/SocketAddress.h>
#include <nlohmann/json.hpp>
#include <optional>
#include <string_view>


namespace brstd {
template <>
class basic_cstring_view<char, ::std::char_traits<char>> {
public:
    ::std::string_view mView;
};
} // namespace brstd

namespace Bedrock::Threading {

template <>
class IAsyncGetResult<::Bedrock::Result<::NetherNet::HttpResponse>> : public IAsyncInfo {
public:
    virtual ::Bedrock::Result<::NetherNet::HttpResponse> getResult() const = 0;
};

template <>
class IAsyncResult<::Bedrock::Result<::NetherNet::HttpResponse>>
: public AsyncBase, public IAsyncGetResult<::Bedrock::Result<::NetherNet::HttpResponse>> {
public:
    using Handle            = ::std::shared_ptr<IAsyncResult>;
    using CompletionHandler = ::std::function<void(IAsyncResult const&)>;

    virtual void addOnComplete(CompletionHandler) = 0;
};

} // namespace Bedrock::Threading

namespace ila::mc::inline server {

void ServerPongBeforeEvent::serialize(CompoundTag& nbt) const {
    Cancellable::serialize(nbt);
    nbt["motd"]            = motd();
    nbt["protocolVersion"] = protocolVersion();
    nbt["networkVersion"]  = networkVersion();
    nbt["playerCount"]     = playerCount();
    nbt["maxPlayerCount"]  = maxPlayerCount();
    nbt["guid"]            = guid();
    nbt["levelName"]       = levelName();
    nbt["gameMode"]        = magic_enum::enum_name(gameMode());
    nbt["localPort"]       = localPort();
    nbt["localPortV6"]     = localPortV6();
    nbt["others"]          = ListTag{};
    for (auto& item : other()) {
        nbt["others"].push_back(item);
    }
    nbt["ipAndPort"] = ipAndPort();
}
void ServerPongBeforeEvent::deserialize(CompoundTag const& nbt) {
    Cancellable::deserialize(nbt);
    motd()            = nbt["motd"];
    protocolVersion() = nbt["protocolVersion"];
    networkVersion()  = nbt["networkVersion"];
    playerCount()     = nbt["playerCount"];
    maxPlayerCount()  = nbt["maxPlayerCount"];
    guid()            = nbt["guid"];
    levelName()       = nbt["levelName"];
    gameMode()        = magic_enum::enum_cast<GameType>(nbt["gameMode"].get<StringTag>()).value_or(gameMode());
    localPort()       = nbt["localPort"];
    localPortV6()     = nbt["localPortV6"];
    other().clear();
    for (auto& item : nbt["others"].get<ListTag>()) {
        other().push_back(item);
    }
}
std::string&              ServerPongBeforeEvent::motd() const { return mMotd; }
int&                      ServerPongBeforeEvent::protocolVersion() const { return mProtocolVersion; }
std::string&              ServerPongBeforeEvent::networkVersion() const { return mNetworkVersion; }
int&                      ServerPongBeforeEvent::playerCount() const { return mPlayerCount; }
int&                      ServerPongBeforeEvent::maxPlayerCount() const { return mMaxPlayerCount; }
std::string&              ServerPongBeforeEvent::guid() const { return mGuid; }
std::string&              ServerPongBeforeEvent::levelName() const { return mLevelName; }
GameType&                 ServerPongBeforeEvent::gameMode() const { return mGameMode; }
ushort&                   ServerPongBeforeEvent::localPort() const { return mLocalPort; }
ushort&                   ServerPongBeforeEvent::localPortV6() const { return mLocalPortV6; }
std::vector<std::string>& ServerPongBeforeEvent::other() const { return mOther; }
std::string const&        ServerPongBeforeEvent::ipAndPort() const { return mIpAndPort; }
std::string               ServerPongBeforeEvent::ip() const {
    auto const address = ipAndPort();
    return address.substr(0, address.find('|'));
}
ushort ServerPongBeforeEvent::port() const {
    auto const address = ipAndPort();
    auto const pos     = address.find('|');
    if (pos == std::string::npos) {
        return 0;
    }
    auto const value = ll::string_utils::svtous(address.substr(pos + 1));
    return value.has_value() ? value.value() : 0;
}

void ServerPongAfterEvent::serialize(CompoundTag& nbt) const {
    Event::serialize(nbt);
    nbt["motd"]            = motd();
    nbt["protocolVersion"] = protocolVersion();
    nbt["networkVersion"]  = networkVersion();
    nbt["playerCount"]     = playerCount();
    nbt["maxPlayerCount"]  = maxPlayerCount();
    nbt["guid"]            = guid();
    nbt["levelName"]       = levelName();
    nbt["gameMode"]        = magic_enum::enum_name(gameMode());
    nbt["localPort"]       = localPort();
    nbt["localPortV6"]     = localPortV6();
    nbt["others"]          = ListTag{};
    for (auto& item : other()) {
        nbt["others"].push_back(item);
    }
    nbt["ipAndPort"] = ipAndPort();
}
std::string const&              ServerPongAfterEvent::motd() const { return mMotd; }
int const&                      ServerPongAfterEvent::protocolVersion() const { return mProtocolVersion; }
std::string const&              ServerPongAfterEvent::networkVersion() const { return mNetworkVersion; }
int const&                      ServerPongAfterEvent::playerCount() const { return mPlayerCount; }
int const&                      ServerPongAfterEvent::maxPlayerCount() const { return mMaxPlayerCount; }
std::string const&              ServerPongAfterEvent::guid() const { return mGuid; }
std::string const&              ServerPongAfterEvent::levelName() const { return mLevelName; }
GameType const&                 ServerPongAfterEvent::gameMode() const { return mGameMode; }
ushort const&                   ServerPongAfterEvent::localPort() const { return mLocalPort; }
ushort const&                   ServerPongAfterEvent::localPortV6() const { return mLocalPortV6; }
std::vector<std::string> const& ServerPongAfterEvent::other() const { return mOther; }
std::string const&              ServerPongAfterEvent::ipAndPort() const { return mIpAndPort; }
std::string                     ServerPongAfterEvent::ip() const {
    auto const address = ipAndPort();
    return address.substr(0, address.find('|'));
}
ushort ServerPongAfterEvent::port() const {
    auto const address = ipAndPort();
    auto const pos     = address.find('|');
    if (pos == std::string::npos) {
        return 0;
    }
    auto const value = ll::string_utils::svtous(address.substr(pos + 1));
    return value.has_value() ? value.value() : 0;
}

LL_TYPE_INSTANCE_HOOK(
    ServerPongEventHook,
    HookPriority::Normal,
    RakNet::RNS2_Windows,
    &RakNet::RNS2_Windows::$Send,
    int,
    RakNet::RNS2_SendParameters* pSendParameters,
    char const*                  pFile,
    uint                         pLine
)
try {
    if (pSendParameters->data[0] != 28) {
        return origin(pSendParameters, pFile, pLine);
    }
    constexpr static int head_size = sizeof(int8) + sizeof(uint64) + sizeof(uint64) + 16;
    auto*                data      = pSendParameters->data;
    size_t               strlen    = data[head_size] << 8 | data[head_size + 1];
    if (static_cast<int>(strlen) != pSendParameters->length - (head_size + 2)) {
        return origin(pSendParameters, pFile, pLine);
    }
    std::istringstream       iss(std::string({data + head_size + 2, strlen}));
    std::string              tmp;
    std::vector<std::string> parts;
    while (std::getline(iss, tmp, ';')) {
        parts.push_back(tmp);
    }
    if (parts.size() < 13) {
        return origin(pSendParameters, pFile, pLine);
    }

    auto                     motd            = parts[1];
    auto                     protocolVersion = std::stoi(parts[2]);
    auto                     networkVersion  = parts[3];
    auto                     playerCount     = std::stoi(parts[4]);
    auto                     maxPlayerCount  = std::stoi(parts[5]);
    auto                     guid            = parts[6];
    auto                     levelName       = parts[7];
    auto                     gameType        = magic_enum::enum_cast<GameType>(parts[8]).value_or(GameType::Survival);
    auto                     localPort       = static_cast<ushort>(std::stoi(parts[10]));
    auto                     localPortV6     = static_cast<ushort>(std::stoi(parts[11]));
    std::vector<std::string> others;
    for (size_t i = 13; i < parts.size(); i++) {
        others.push_back(parts[i]);
    }

    std::string ipAndPort;
    ipAndPort.resize(56);
    pSendParameters->systemAddress->ToString(true, ipAndPort.data(), '|');

    auto beforeEvent = ServerPongBeforeEvent(
        motd,
        protocolVersion,
        networkVersion,
        playerCount,
        maxPlayerCount,
        guid,
        levelName,
        gameType,
        localPort,
        localPortV6,
        others,
        ipAndPort
    );
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) {
        return 133;
    }

    auto text = fmt::format(
        "MCPE;{};{};{};{};{};{};{};{};1;{};{};0;",
        motd,
        protocolVersion,
        networkVersion,
        playerCount,
        maxPlayerCount,
        guid,
        levelName,
        magic_enum::enum_name(gameType),
        localPort,
        localPortV6
    );
    for (auto& other : others) {
        text += other + ";";
    }

    std::vector<char> packet;
    packet.reserve(256);
    packet.insert(packet.end(), data, data + head_size);
    strlen = text.length();
    packet.push_back(static_cast<char>((strlen >> 8) & 0xFF));
    packet.push_back(static_cast<char>(strlen & 0xFF));
    packet.insert(packet.end(), text.begin(), text.end());
    pSendParameters->data   = packet.data();
    pSendParameters->length = static_cast<int>(packet.size());

    auto result = origin(pSendParameters, pFile, pLine);
    LLEventBus.publish(ServerPongAfterEvent(
        motd,
        protocolVersion,
        networkVersion,
        playerCount,
        maxPlayerCount,
        guid,
        levelName,
        gameType,
        localPort,
        localPortV6,
        others,
        ipAndPort
    ));
    return result;
} catch (...) {
    return origin(pSendParameters, pFile, pLine);
}

namespace {

struct NetherNetServerData {
    static constexpr uchar VERSION_NUMBER = 7;

    uchar       version = VERSION_NUMBER;
    std::string name;
    int         protocol = 0;
    std::string appVersion;
    std::string levelName;
    int         players        = 0;
    int         maxPlayers     = 0;
    int         gameType       = 0;
    bool        editorWorld    = false;
    bool        hardcore       = false;
    bool        onlineAuth     = false;
    bool        selfSignedAuth = false;
    std::string nonce;
    int         connectionType = 0;

    Bedrock::Result<void> read(ReadOnlyBinaryStream& stream, std::size_t maxLength) {
        auto dataVersion = stream.getByte();
        if (!dataVersion) {
            return nonstd::make_unexpected(dataVersion.error());
        }
        version = dataVersion.value();
        if (version != VERSION_NUMBER) {
            return nonstd::make_unexpected(
                ::Bedrock::ErrorInfo<::std::error_code>{std::make_error_code(std::errc::not_supported)}
            );
        }
        auto nameResult = stream.getString(maxLength);
        if (!nameResult) {
            return nonstd::make_unexpected(nameResult.error());
        }
        name = std::move(nameResult).value();

        auto protocolResult = stream.getVarInt64();
        if (!protocolResult) {
            return nonstd::make_unexpected(protocolResult.error());
        }
        protocol = static_cast<int>(protocolResult.value());

        auto appVersionResult = stream.getString(maxLength);
        if (!appVersionResult) {
            return nonstd::make_unexpected(appVersionResult.error());
        }
        appVersion = std::move(appVersionResult).value();

        auto levelNameResult = stream.getString(maxLength);
        if (!levelNameResult) {
            return nonstd::make_unexpected(levelNameResult.error());
        }
        levelName = std::move(levelNameResult).value();

        auto playersResult = stream.getVarInt64();
        if (!playersResult) {
            return nonstd::make_unexpected(playersResult.error());
        }
        players = static_cast<int>(playersResult.value());

        auto maxPlayersResult = stream.getVarInt64();
        if (!maxPlayersResult) {
            return nonstd::make_unexpected(maxPlayersResult.error());
        }
        maxPlayers = static_cast<int>(maxPlayersResult.value());

        auto gameTypeResult = stream.getVarInt64();
        if (!gameTypeResult) {
            return nonstd::make_unexpected(gameTypeResult.error());
        }
        gameType = static_cast<int>(gameTypeResult.value());

        auto editorResult = stream.getBool();
        if (!editorResult) {
            return nonstd::make_unexpected(editorResult.error());
        }
        editorWorld = editorResult.value();

        auto hardcoreResult = stream.getBool();
        if (!hardcoreResult) {
            return nonstd::make_unexpected(hardcoreResult.error());
        }
        hardcore = hardcoreResult.value();

        auto onlineAuthResult = stream.getBool();
        if (!onlineAuthResult) {
            return nonstd::make_unexpected(onlineAuthResult.error());
        }
        onlineAuth = onlineAuthResult.value();

        auto selfSignedAuthResult = stream.getBool();
        if (!selfSignedAuthResult) {
            return nonstd::make_unexpected(selfSignedAuthResult.error());
        }
        selfSignedAuth = selfSignedAuthResult.value();

        auto nonceResult = stream.getString(maxLength);
        if (!nonceResult) {
            return nonstd::make_unexpected(nonceResult.error());
        }
        nonce = std::move(nonceResult).value();

        auto connectionResult = stream.getVarInt64();
        if (!connectionResult) {
            return nonstd::make_unexpected(connectionResult.error());
        }
        connectionType = static_cast<int>(connectionResult.value());
        return {};
    }

    void write(BinaryStream& stream) const {
        stream.writeByte(VERSION_NUMBER, "dataVersion", nullptr);
        stream.writeString(name, "name", nullptr);
        stream.writeVarInt(protocol, "protocol", nullptr);
        stream.writeString(appVersion, "version", nullptr);
        stream.writeString(levelName, "level", nullptr);
        stream.writeVarInt(players, "players", nullptr);
        stream.writeVarInt(maxPlayers, "maxPlayers", nullptr);
        stream.writeVarInt(gameType, "gameType", nullptr);
        stream.writeBool(editorWorld, "editor", nullptr);
        stream.writeBool(hardcore, "hardcore", nullptr);
        stream.writeBool(onlineAuth, "onlineAuth", nullptr);
        stream.writeBool(selfSignedAuth, "selfSignedAuth", nullptr);
        stream.writeString(nonce, "nonce", nullptr);
        stream.writeVarInt(connectionType, "connection", nullptr);
    }
};

constexpr std::size_t netherNetServerDataMaxHexLength = 1148;

std::optional<std::string> netherNetServerDataFromHex(std::string_view hex) {
    if (hex.size() % 2 != 0) {
        return std::nullopt;
    }
    std::string bytes(hex.size() / 2, '\0');
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        auto const digit = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        };
        auto const high = digit(hex[i * 2]);
        auto const low  = digit(hex[i * 2 + 1]);
        if (high < 0 || low < 0) {
            return std::nullopt;
        }
        bytes[i] = static_cast<char>(high << 4 | low);
    }
    return bytes;
}

std::string netherNetServerDataToHex(std::string_view bytes) {
    constexpr std::string_view digits = "0123456789abcdef";
    std::string                hex;
    hex.reserve(bytes.size() * 2);
    for (auto const byte : bytes) {
        auto const value  = static_cast<uchar>(byte);
        hex              += digits[value >> 4];
        hex              += digits[value & 0xF];
    }
    return hex;
}

} // namespace

LL_TYPE_INSTANCE_HOOK(
    NetherNetServerPongEventHook,
    HookPriority::Normal,
    NetherNet::LanThreadManager,
    &NetherNet::LanThreadManager::$SendLanBroadcastResponse,
    void,
    webrtc::SocketAddress const& destination,
    NetherNet::NetworkID         from,
    std::string                  data
) {
    auto                 bytes = netherNetServerDataFromHex(data).value_or("");
    ReadOnlyBinaryStream input(bytes, false);

    NetherNetServerData serverData;
    if (!serverData.read(input, bytes.size())) {
        return origin(destination, from, std::move(data));
    }

    std::string ipAndPort = destination.HostAsURIString() + '|' + std::to_string(destination.port());
    std::string guid      = from.toString();
    ushort      localPort = 0;
    if (auto settings = ll::service::getPropertiesSettings()) {
        localPort = settings->mServerPort;
    }
    ushort                   localPortV6 = 0;
    std::vector<std::string> others;
    auto                     gameMode = static_cast<GameType>(serverData.gameType);

    auto beforeEvent = ServerPongBeforeEvent(
        serverData.name,
        serverData.protocol,
        serverData.appVersion,
        serverData.players,
        serverData.maxPlayers,
        guid,
        serverData.levelName,
        gameMode,
        localPort,
        localPortV6,
        others,
        ipAndPort
    );
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) {
        return;
    }

    serverData.name       = beforeEvent.motd();
    serverData.protocol   = beforeEvent.protocolVersion();
    serverData.appVersion = beforeEvent.networkVersion();
    serverData.players    = beforeEvent.playerCount();
    serverData.maxPlayers = beforeEvent.maxPlayerCount();
    serverData.levelName  = beforeEvent.levelName();
    serverData.gameType   = static_cast<int>(gameMode);

    BinaryStream output;
    serverData.write(output);
    auto response = netherNetServerDataToHex(output.mBuffer);
    if (response.size() > netherNetServerDataMaxHexLength) {
        SelfLogger.warn(
            "NetherNet discovery response is {} characters, exceeding the {} character limit, keeping the original",
            response.size(),
            netherNetServerDataMaxHexLength
        );
        return origin(destination, from, std::move(data));
    }

    origin(destination, from, std::move(response));
    LLEventBus.publish(ServerPongAfterEvent(
        serverData.name,
        serverData.protocol,
        serverData.appVersion,
        serverData.players,
        serverData.maxPlayers,
        guid,
        serverData.levelName,
        gameMode,
        localPort,
        localPortV6,
        others,
        ipAndPort
    ));
}


namespace {
thread_local ::webrtc::Socket* netherNetHttpSocket = nullptr;
} // namespace

LL_TYPE_INSTANCE_HOOK(
    NetherNetHttpConnectionReadHook,
    HookPriority::Normal,
    NetherNet::HttpConnection,
    &NetherNet::HttpConnection::_onReadEvent,
    void,
    ::webrtc::Socket* socket
) {
    netherNetHttpSocket = socket;
    origin(socket);
}

LL_TYPE_INSTANCE_HOOK(
    NetherNetHttpJoinInfoHook,
    HookPriority::Normal,
    NetherNet::HttpSignalingServer,
    &NetherNet::HttpSignalingServer::$onRequest,
    ::Bedrock::Threading::Async<::Bedrock::Result<::NetherNet::HttpResponse>>,
    ::NetherNet::HttpRequest request
) {
    class JoinAsyncResult : public ::Bedrock::Threading::IAsyncResult<::Bedrock::Result<::NetherNet::HttpResponse>> {
    public:
        using Value   = ::Bedrock::Result<::NetherNet::HttpResponse>;
        using Handler = IAsyncResult::CompletionHandler;

        explicit JoinAsyncResult(Value value) : mValue(std::move(value)) {}

        ::Bedrock::Threading::AsyncStatus getStatus() const override {
            return ::Bedrock::Threading::AsyncStatus::Completed;
        }
        ::std::error_code getError() const override { return {}; }
        void              cancel() override {}
        Value             getResult() const override { return mValue; }
        void              addOnComplete(Handler handler) override {
            if (handler) {
                handler(*this);
            }
        }

    private:
        Value mValue;
    };

    bool const isJoinInfo = request.path.get() == "/v1/join";
    auto       async      = origin(std::move(request));
    if (!isJoinInfo || async.mResult == nullptr) {
        return async;
    }
    auto result = async.mResult->getResult();
    if (!result.has_value()) {
        return async;
    }
    auto& response = result.value();
    if (response.statusCode != 200 || response.contentType.get().mView != "application/json") {
        return async;
    }
    auto& body = response.body.get();
    auto  json = nlohmann::ordered_json::parse(body, nullptr, false);
    if (json.is_discarded()) {
        return async;
    }

    ushort localPort = 0;
    if (netherNetHttpSocket) {
        localPort = netherNetHttpSocket->GetLocalAddress().port();
    }

    std::string              motd        = json.value("name", std::string{});
    int                      protocol    = json.value("protocol", 0);
    std::string              version     = json.value("version", std::string{});
    std::string              levelName   = json.value("level", std::string{});
    int                      playerCount = json.value("players", 0);
    int                      maxPlayers  = json.value("maxPlayers", 0);
    auto                     gameMode    = static_cast<GameType>(json.value("gameType", 0));
    ushort                   localPortV6 = 0;
    std::vector<std::string> other;
    std::string              ipAndPort;
    if (netherNetHttpSocket) {
        auto const address = netherNetHttpSocket->GetRemoteAddress();
        ipAndPort          = address.HostAsURIString() + '|' + std::to_string(address.port());
    }
    std::string guid;

    auto beforeEvent = ServerPongBeforeEvent(
        motd,
        protocol,
        version,
        playerCount,
        maxPlayers,
        guid,
        levelName,
        gameMode,
        localPort,
        localPortV6,
        other,
        ipAndPort
    );
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) {
        response.statusCode             = 404;
        response.statusText.get().mView = "Not Found";
        response.headers.get().clear();
        body.clear();
    } else {
        json["name"]       = motd;
        json["protocol"]   = protocol;
        json["version"]    = version;
        json["level"]      = levelName;
        json["players"]    = playerCount;
        json["maxPlayers"] = maxPlayers;
        json["gameType"]   = static_cast<int>(gameMode);
        body               = json.dump();

        LLEventBus.publish(ServerPongAfterEvent(
            motd,
            protocol,
            version,
            playerCount,
            maxPlayers,
            guid,
            levelName,
            gameMode,
            localPort,
            localPortV6,
            other,
            ipAndPort
        ));
    }
    async.mResult = std::make_shared<JoinAsyncResult>(std::move(result));
    return async;
}

Event_Hook_Factory(
    ServerPong,
    <ServerPongEventHook, NetherNetServerPongEventHook, NetherNetHttpConnectionReadHook, NetherNetHttpJoinInfoHook>
);

} // namespace ila::mc::inline server
