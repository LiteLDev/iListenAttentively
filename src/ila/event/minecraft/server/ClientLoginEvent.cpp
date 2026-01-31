#include "ila/event/minecraft/server/ClientLoginEvent.h"
#include "ila/base/Gloabl.h"
#include "ila/event/minecraft/server/ReceivePacketEvent.h"
#include <ll/api/service/Bedrock.h>
#include <mc/certificates/UnverifiedCertificate.h>
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
    nbt["serverNetworkHandler"] = serializeRefObj(mServerNetworkHandler);
    nbt["networkIdentifier"]    = serializeRefObj(mNetworkIdentifier);
}

void ClientLoginAfterEvent::serialize(CompoundTag& nbt) const
{
    Event::serialize(nbt);
    nbt["serverNetworkHandler"] = serializeRefObj(mServerNetworkHandler);
    nbt["networkIdentifier"]    = serializeRefObj(mNetworkIdentifier);
    nbt["uuid"]                 = mUuid.asString();
    nbt["serverAuthXuid"]       = mServerAuthXuid;
    nbt["clientAuthXuid"]       = mClientAuthXuid;
    nbt["realName"]             = mRealName;
    nbt["ipAndPort"]            = mIpAndPort;
}
std::string              ClientLoginAfterEvent::ip() const
{
    auto address = mIpAndPort;
    return address.substr(0, address.find("|"));
}
std::string ClientLoginAfterEvent::port() const
{
    auto address = mIpAndPort;
    return address.substr(address.find("|") + 1);
}
void ClientLoginAfterEvent::disConnectClient(std::string const& reason) const
{
    if (!mKickReasons) mKickReasons.emplace();
    if (!reason.empty()) mKickReasons->emplace_back(reason);
}

LL_TYPE_INSTANCE_HOOK(
    ClientLoginEventHook,
    HookPriority::Normal,
    ServerNetworkHandler,
    &ServerNetworkHandler::$handle,
    void,
    NetworkIdentifier const&     pSource,
    std::shared_ptr<LoginPacket> pPacket
)
{
    auto beforeEvent = ClientLoginBeforeEvent(*thisFor<NetEventCallback>(), pSource);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled() || true)
    {
        thisFor<NetEventCallback>()
            ->disconnectClient(pSource, pPacket->mSenderSubId, Connection::DisconnectFailReason::Kicked);
        return;
    }
    origin(pSource, pPacket);
    auto client = thisFor<NetEventCallback>()->mClients->find(pSource);
    if (client == mClients->end()) { return; }
    auto&                                   info = *client->second->mPrimaryPlayerInfo;
    std::optional<std::vector<std::string>> kickReasons;
    auto                                    afterEvent = ClientLoginAfterEvent(
        *thisFor<NetEventCallback>(),
        pSource,
        info.AuthenticatedUuid,
        info.Xuid,
        info.Xuid,
        info.XboxLiveName,
        pSource.getIPAndPort(),
        kickReasons
    );
    LLEventBus.publish(afterEvent);
    if (kickReasons)
    {
        thisFor<NetEventCallback>()->disconnectClientWithMessage(
            pSource,
            pPacket->mSenderSubId,
            Connection::DisconnectFailReason::Kicked,
            fmt::to_string(fmt::join(*kickReasons, "§r\n")),
            std::nullopt,
            false
        );
    }
}

Event_Hook_Factory(ClientLogin, <ClientLoginEventHook>);

} // namespace ila::mc::inline server