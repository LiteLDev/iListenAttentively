#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <mc/world/level/GameType.h>

namespace ila::mc::inline server
{
class ServerPongBeforeEvent final : public ll::event::Cancellable<ll::event::Event>
{
public:
    std::string&              mMotd;
    int&                      mProtocolVersion;
    std::string&              mNetworkVersion;
    int&                      mPlayerCount;
    int&                      mMaxPlayerCount;
    std::string&              mGuid;
    std::string&              mLevelName;
    GameType&                 mGameMode;
    ushort&                   mLocalPort;
    ushort&                   mLocalPortV6;
    std::vector<std::string>& mOther;
    std::string const&        mIpAndPort;

public:
    constexpr explicit ServerPongBeforeEvent(
        std::string&              motd,
        int&                      protocolVersion,
        std::string&              networkVersion,
        int&                      playerCount,
        int&                      maxPlayerCount,
        std::string&              guid,
        std::string&              levelName,
        GameType&                 gameMode,
        ushort&                   localPort,
        ushort&                   localPortV6,
        std::vector<std::string>& other,
        std::string const&        ipAndPort
    )
        : mMotd(motd)
        , mProtocolVersion(protocolVersion)
        , mNetworkVersion(networkVersion)
        , mPlayerCount(playerCount)
        , mMaxPlayerCount(maxPlayerCount)
        , mGuid(guid)
        , mLevelName(levelName)
        , mGameMode(gameMode)
        , mLocalPort(localPort)
        , mLocalPortV6(localPortV6)
        , mOther(other)
        , mIpAndPort(ipAndPort)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

    ILNDAPI std::string ip() const;
    ILNDAPI ushort      port() const;
}; // class ServerPongEvent

class ServerPongAfterEvent final : public ll::event::Event
{
public:
    std::string const&       mMotd;
    int const&               mProtocolVersion;
    std::string const&       mNetworkVersion;
    int const&               mPlayerCount;
    int const&               mMaxPlayerCount;
    std::string const&       mGuid;
    std::string const&       mLevelName;
    GameType const&          mGameMode;
    ushort const&            mLocalPort;
    ushort const&            mLocalPortV6;
    std::vector<std::string> mOther;
    std::string const&       mIpAndPort;

public:
    constexpr explicit ServerPongAfterEvent(
        std::string const&              motd,
        int const&                      protocolVersion,
        std::string&                    networkVersion,
        int const&                      playerCount,
        int const&                      maxPlayerCount,
        std::string const&              guid,
        std::string const&              levelName,
        GameType const&                 gameMode,
        ushort const&                   localPort,
        ushort const&                   localPortV6,
        std::vector<std::string> const& other,
        std::string const&              ipAndPort
    )
        : mMotd(motd)
        , mProtocolVersion(protocolVersion)
        , mNetworkVersion(networkVersion)
        , mPlayerCount(playerCount)
        , mMaxPlayerCount(maxPlayerCount)
        , mGuid(guid)
        , mLevelName(levelName)
        , mGameMode(gameMode)
        , mLocalPort(localPort)
        , mLocalPortV6(localPortV6)
        , mOther(other)
        , mIpAndPort(ipAndPort)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

    ILNDAPI std::string ip() const;
    ILNDAPI ushort      port() const;
}; // class ServerPongEvent
} // namespace ila::mc::inline server