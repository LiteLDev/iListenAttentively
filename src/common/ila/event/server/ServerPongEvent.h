#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <mc/world/level/GameType.h>
#include <utility>

namespace ila::mc::inline server {

class ServerPongEvent : public ll::event::Event {
public:
    struct PongData {
        std::string              mMotd;
        int                      mNetworkProtocolVersion;
        std::string              mMinecraftVersionNetwork;
        int                      mNumPlayers;
        int                      mMaxPlayers;
        std::string              mServerGuid;
        std::string              mLevelName;
        GameType                 mGameMode;
        ushort                   mLocalPort;
        ushort                   mLocalPortV6;
        std::vector<std::string> mOthers;
    };

private:
    std::pair<std::string, ushort> mAddress;
    PongData&                      mPongData;

public:
    constexpr explicit ServerPongEvent(std::pair<std::string, ushort> address, PongData& pongData)
    : Event(),
      mAddress(address),
      mPongData(pongData) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

public:
    std::pair<std::string, ushort> const& address() const { return mAddress; }
    std::string&                          motd() const { return mPongData.mMotd; }
    int                                   networkProtocolVersion() const { return mPongData.mNetworkProtocolVersion; }
    std::string&                          minecraftVersionNetwork() const { return mPongData.mMinecraftVersionNetwork; }
    int                                   numPlayers() const { return mPongData.mNumPlayers; }
    int                                   maxPlayers() const { return mPongData.mMaxPlayers; }
    std::string&                          serverGuid() const { return mPongData.mServerGuid; }
    std::string&                          levelName() const { return mPongData.mLevelName; }
    GameType&                             gameMode() const { return mPongData.mGameMode; }
    int                                   localPort() const { return mPongData.mLocalPort; }
    int                                   localPortV6() const { return mPongData.mLocalPortV6; }
    std::vector<std::string>&             others() const { return mPongData.mOthers; }
};

class SendingServerPongEvent final : public ll::event::Cancellable<ServerPongEvent> {
public:
    using Cancellable::Cancellable;
};

class SentServerPongEvent final : public ServerPongEvent {
public:
    using ServerPongEvent::ServerPongEvent;
};

} // namespace ila::mc::inline server