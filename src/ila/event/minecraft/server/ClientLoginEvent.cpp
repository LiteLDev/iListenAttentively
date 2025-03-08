#include "ila/event/minecraft/server/ClientLoginEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/service/Bedrock.h>
#include <mc/certificates/identity/GameServerToken.h>
#include <mc/network/ConnectionRequest.h>
#include <mc/network/packet/LoginPacket.h>

namespace ila::mc::inline server
{

void ClientLoginBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["serverNetworkHandler"] = serializeRefObj(serverNetworkHandler());
    nbt["networkIdentifier"]    = serializeRefObj(networkIdentifier());
}
ServerNetworkHandler const& ClientLoginBeforeEvent::serverNetworkHandler() const
{
    return mServerNetworkHandler;
}
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
ServerNetworkHandler const& ClientLoginAfterEvent::serverNetworkHandler() const
{
    return mServerNetworkHandler;
}
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
void ClientLoginAfterEvent::disConnectClient(std::string reason) const
{
    ll::service::getServerNetworkHandler()->disconnectClient(
        networkIdentifier(),
        Connection::DisconnectFailReason::Kicked,
        reason,
        std::nullopt,
        false
    );
}

LL_TYPE_INSTANCE_HOOK(
    ClientLoginEventHook,
    HookPriority::Normal,
    ServerNetworkHandler,
    &ServerNetworkHandler::$handle,
    void,
    NetworkIdentifier const& pSource,
    LoginPacket const&       pPacket
)
{
    auto beforeEvent = ClientLoginBeforeEvent(*this, pSource);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pSource, pPacket);
    auto& cert = pPacket.mConnectionRequest->mGameServerToken;
    LLEventBus.publish(ClientLoginAfterEvent(
        *this,
        pSource,
        cert->getIdentity(),
        cert->getXuid(false),
        cert->getXuid(true),
        cert->getIdentityName(),
        pSource.getIPAndPort()
    ));
}

Event_Hook_Factory(ClientLogin, <ClientLoginEventHook>);

} // namespace ila::mc::inline server