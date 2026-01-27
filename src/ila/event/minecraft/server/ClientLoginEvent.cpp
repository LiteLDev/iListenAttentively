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
    if (beforeEvent.isCancelled()) { return; }
    origin(pSource, pPacket);
    auto& data = pPacket->mConnectionRequest->mCertificateData->mRawToken.mDataInfo;
    if (!data.isMember("extraData") || !data["extraData"].isObject()) { return; }
    auto&                                   extraData = data["extraData"];
    std::optional<std::vector<std::string>> kickReasons;
    // TODO: 这里的XUID获取其实有问题，需要判断一个条件
    auto                                    afterEvent = ClientLoginAfterEvent(
        *this,
        pSource,
        extraData.isMember("identity") ? mce::UUID::fromString(extraData["identity"].asString(""))
                                                                          : mce::UUID::EMPTY(),
        extraData.isMember("XUID") ? extraData["XUID"].asString("") : "",
        extraData.isMember("XUID") ? extraData["XUID"].asString("") : "",
        extraData.isMember("displayName") ? extraData["displayName"].asString("") : "",
        pSource.getIPAndPort(),
        kickReasons
    );
    LLEventBus.publish(afterEvent);
    if (kickReasons)
    {
        thisFor<NetEventCallback>()->disconnectClientWithMessage(
            afterEvent.networkIdentifier(),
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