#include "ila/event/minecraft/server/ServerPongEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/Versions.h>
#include <mc/deps/raknet/RNS2_SendParameters.h>
#include <mc/deps/raknet/RNS2_Windows_Linux_360.h>
#include <mc/deps/raknet/SystemAddress.h>

namespace ila::mc::inline server
{

void ServerPongBeforeEvent::serialize(CompoundTag& nbt) const
{
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
    nbt["others"]          = ListTag {};
    for (auto& item : other()) { nbt["others"].push_back(item); }
}
void ServerPongBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    motd()            = nbt["motd"];
    protocolVersion() = nbt["protocolVersion"];
    networkVersion()  = nbt["networkVersion"];
    playerCount()     = nbt["playerCount"];
    maxPlayerCount()  = nbt["maxPlayerCount"];
    guid()            = nbt["guid"];
    levelName()       = nbt["levelName"];
    gameMode()    = magic_enum::enum_cast<GameType>(nbt["gameMode"].get<StringTag>()).value_or(gameMode());
    localPort()   = nbt["localPort"];
    localPortV6() = nbt["localPortV6"];
    other().clear();
    for (auto& item : nbt["others"].get<ListTag>()) { other().push_back(item); }
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

void ServerPongAfterEvent::serialize(CompoundTag& nbt) const
{
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

LL_STATIC_HOOK(
    ServerPongEventHook,
    HookPriority::Normal,
    &RakNet::RNS2_Windows_Linux_360::Send_Windows_Linux_360NoVDP,
    int,
    int                          pRns2Socket,
    RakNet::RNS2_SendParameters* pSendParameters,
    char const*                  pFile,
    uint                         pLine
)
{
    if (pSendParameters->mUnk98c838.as<char*>()[0] == 28)
    {
        constexpr static int head_size = sizeof(char) + sizeof(std::uint64_t) + sizeof(std::uint64_t) + 16;
        const char*          data      = pSendParameters->mUnk98c838.as<char*>();
        std::size_t          strlen    = data[head_size] << 8 | data[head_size + 1];
        if (strlen == 0) { return origin(pRns2Socket, pSendParameters, pFile, pLine); }
        std::istringstream       iss(std::string({ data + head_size + 2, strlen }));
        std::string              tmp;
        std::vector<std::string> parts;
        while (std::getline(iss, tmp, ';')) { parts.push_back(tmp); }
        if (parts.size() != 13) { return origin(pRns2Socket, pSendParameters, pFile, pLine); }

        std::string motd            = parts[1];
        int         protocolVersion = std::stoi(parts[2]);
        std::string networkVersion  = parts[3];
        int         playerCount     = std::stoi(parts[4]);
        int         maxPlayerCount  = std::stoi(parts[5]);
        std::string guid            = parts[6];
        std::string levelName       = parts[7];
        GameType    mGameType       = magic_enum::enum_cast<GameType>(parts[8]).value_or(GameType::Survival);
        ushort      mLocalPort      = static_cast<ushort>(std::stoi(parts[10]));
        ushort      mLocalPortV6    = static_cast<ushort>(std::stoi(parts[11]));
        std::vector<std::string> mOther = { "LeviLamina" };
        for (size_t i = 13; i < parts.size(); i++) { mOther.push_back(parts[i]); }

        auto beforeEvent = ServerPongBeforeEvent(
            motd,
            protocolVersion,
            networkVersion,
            playerCount,
            maxPlayerCount,
            guid,
            levelName,
            mGameType,
            mLocalPort,
            mLocalPortV6,
            mOther
        );
        LLEventBus.publish(beforeEvent);
        if (beforeEvent.isCancelled()) { return 133; }

        std::string text = fmt::format(
            "MCPE;{};{};{};{};{};{};{};{};1;{};{};0;",
            motd,
            protocolVersion,
            networkVersion,
            playerCount,
            maxPlayerCount,
            guid,
            levelName,
            magic_enum::enum_name(mGameType),
            mLocalPort,
            mLocalPortV6
        );
        for (auto& other : mOther) { text += other + ";"; }

        std::vector<char> packet;
        packet.reserve(256);
        packet.insert(packet.end(), data, data + head_size);
        strlen = text.length();
        packet.push_back(static_cast<char>((strlen >> 8) & 0xFF));
        packet.push_back(static_cast<char>(strlen & 0xFF));
        packet.insert(packet.end(), text.begin(), text.end());
        pSendParameters->mUnk98c838.as<char*>() = packet.data();
        pSendParameters->mUnke627d8.as<int>()   = static_cast<int>(packet.size());

        auto result = origin(pRns2Socket, pSendParameters, pFile, pLine);
        LLEventBus.publish(ServerPongAfterEvent(
            motd,
            protocolVersion,
            networkVersion,
            playerCount,
            maxPlayerCount,
            guid,
            levelName,
            mGameType,
            mLocalPort,
            mLocalPortV6,
            mOther
        ));
        return result;
    }
    return origin(pRns2Socket, pSendParameters, pFile, pLine);
}

Event_Hook_Factory(ServerPong, <ServerPongEventHook>);

} // namespace ila::mc::inline server