#include "ila/event/minecraft/server/ClientLoginEvent.h"
#include "ila/base/Gloabl.h"
#include "ila/event/minecraft/server/ReceivePacketEvent.h"
#include <ll/api/service/Bedrock.h>
#include <mc/certificates/identity/GameServerToken.h>
#include <mc/network/ConnectionRequest.h>
#include <mc/network/NetworkIdentifier.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/network/packet/LoginPacket.h>
#include <mc/platform/UUID.h>

namespace ila::mc::inline server
{

void ClientLoginBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["serverNetworkHandler"] = serializeRefObj(serverNetworkHandler());
    nbt["networkIdentifier"]    = serializeRefObj(networkIdentifier());
}
ServerNetworkHandler& ClientLoginBeforeEvent::serverNetworkHandler() const { return mServerNetworkHandler; }
NetworkIdentifier const& ClientLoginBeforeEvent::networkIdentifier() const { return mNetworkIdentifier; }

void ClientLoginAfterEvent::serialize(CompoundTag& nbt) const
{
    Event::serialize(nbt);
    nbt["serverNetworkHandler"] = serializeRefObj(serverNetworkHandler());
    nbt["networkIdentifier"]    = serializeRefObj(networkIdentifier());
    nbt["uuid"]                 = uuid().asString();
    nbt["serverAuthXuid"]       = serverAuthXuid();
    nbt["clientAuthXuid"]       = clientAuthXuid();
    nbt["realName"]             = realName();
    nbt["ipAndPort"]            = ipAndPort();
}
ServerNetworkHandler&    ClientLoginAfterEvent::serverNetworkHandler() const { return mServerNetworkHandler; }
NetworkIdentifier const& ClientLoginAfterEvent::networkIdentifier() const { return mNetworkIdentifier; }
mce::UUID const&         ClientLoginAfterEvent::uuid() const { return mUuid; }
std::string const&       ClientLoginAfterEvent::serverAuthXuid() const { return mServerAuthXuid; }
std::string const&       ClientLoginAfterEvent::clientAuthXuid() const { return mClientAuthXuid; }
std::string const&       ClientLoginAfterEvent::realName() const { return mRealName; }
std::string const&       ClientLoginAfterEvent::ipAndPort() const { return mIpAndPort; }
std::string              ClientLoginAfterEvent::ip() const
{
    auto address = ipAndPort();
    return address.substr(0, address.find("|"));
}
std::string ClientLoginAfterEvent::port() const
{
    auto address = ipAndPort();
    return address.substr(address.find("|") + 1);
}
void ClientLoginAfterEvent::disConnectClient(std::string const& reason) const
{
    if (!mKickReasons) mKickReasons.emplace();
    if (!reason.empty()) mKickReasons->emplace_back(reason);
}

Event_Listener_Factory(ClientLoginBefore)
{
    mListeners.emplace_back(LLEventBus.emplaceListener<ila::mc::ReceivePacketBeforeEvent<LoginPacket>>(
        [](ila::mc::ReceivePacketBeforeEvent<LoginPacket>& event) -> void {
            auto beforeEvent =
                ClientLoginBeforeEvent(*ll::service::getServerNetworkHandler(), event.networkIdentifier());
            LLEventBus.publish(beforeEvent);
            if (beforeEvent.isCancelled()) { event.cancel(); }
        }
    ));
}

Event_Listener_Factory(ClientLoginAfter)
{
    mListeners.emplace_back(LLEventBus.emplaceListener<ila::mc::ReceivePacketAfterEvent<LoginPacket>>(
        [](ila::mc::ReceivePacketAfterEvent<LoginPacket>& event) -> void {
            auto& cert = event.packet().mConnectionRequest->mGameServerToken;
            std::optional<std::vector<std::string>> kickReasons;
            LLEventBus.publish(ClientLoginAfterEvent(
                *ll::service::getServerNetworkHandler(),
                event.networkIdentifier(),
                cert->getIdentity(),
                cert->getXuid(false),
                cert->getXuid(true),
                cert->getIdentityName(),
                event.networkIdentifier().getIPAndPort(),
                kickReasons
            ));
            if (kickReasons)
            {
                ll::service::getServerNetworkHandler()->disconnectClient(
                    event.networkIdentifier(),
                    Connection::DisconnectFailReason::Kicked,
                    fmt::to_string(fmt::join(*kickReasons, "§r\n\n")),
                    std::nullopt,
                    false
                );
            }
        }
    ));
}

} // namespace ila::mc::inline server