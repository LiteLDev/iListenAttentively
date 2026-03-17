#include "ila/event/server/ClientLoginEvent.h"
#include "ila/base/Gloabl.i.h"
#include "mc/network/connection/DisconnectFailReason.h"
#include "patch_mc/platform/UUID.i.h"
#include <ll/api/event/Event.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>
#include <ll/api/service/Bedrock.h>
#include <mc/certificates/identity/PlayerAuthenticationInfo.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/network/NetEventCallback.h>
#include <mc/network/NetworkConnection.h>
#include <mc/network/NetworkIdentifier.h>
#include <mc/network/NetworkSystem.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/network/packet/LoginPacket.h>
#include <mc/platform/UUID.h>
#include <optional>
#include <string>

namespace ila::server {

struct PlayerAuthenticationInfoReflection {
    std::string xuid;
    std::string play_fab_id;
    std::string nintendo_id;
    std::string psn_id;
    std::string xbox_live_name;
    std::string nintendo_name;
    std::string play_station_name;
    std::string public_key;
    mce::UUID   authenticated_uuid;
};
static_assert(
    sizeof(PlayerAuthenticationInfo) == sizeof(PlayerAuthenticationInfoReflection),
    "Explosion and ExplosionReflection are not the same size"
);
static_assert(
    alignof(PlayerAuthenticationInfo) == alignof(PlayerAuthenticationInfoReflection),
    "Explosion and ExplosionReflection are not the same alignment"
);

void ClientLoginEvent::serialize(CompoundTag& nbt) const {
    Event::serialize(nbt);
    nbt["network_identifier"] = serializeRefObj(mNetworkIdentifier);
    reflection::serialize_to(nbt["auth_info"], reinterpret_cast<PlayerAuthenticationInfoReflection&>(mAuthInfo))
        .value();
}

void ClientLoginEvent::deserialize(CompoundTag const& nbt) {
    Event::deserialize(nbt);
    reflection::deserialize(reinterpret_cast<PlayerAuthenticationInfoReflection&>(mAuthInfo), nbt["auth_info"]).value();
}

bool isDisconnect(NetworkIdentifier const& source) {
    auto networkSystem = ll::service::getNetworkSystem(false);
    if (!networkSystem) return true;

    auto* connection = networkSystem->getConnectionFromId(source);
    if (!connection) return true;

    return connection->mShouldCloseConnection;
}

LL_TYPE_INSTANCE_HOOK(
    ClientLoginEventHook1,
    HookPriority::Normal,
    ServerNetworkHandler,
    &ServerNetworkHandler::$_validateLoginPacket,
    std::optional<PlayerAuthenticationInfo>,
    NetworkIdentifier const& source,
    LoginPacket const&       packet
) {
    auto result = origin(source, packet);
    if (!result || isDisconnect(source)) return result;

    auto event = ClientLoginingEvent{source, *result};
    getLLEventBus().publish(event);
    if (event.isCancelled()) {
        thisFor<NetEventCallback>()->disconnectPrimaryClient(source, Connection::DisconnectFailReason::Kicked);
        return std::nullopt;
    }

    return result;
}

LL_TYPE_INSTANCE_HOOK(
    ClientLoginEventHook2,
    HookPriority::Normal,
    ServerNetworkHandler,
    &ServerNetworkHandler::_onClientAuthenticated,
    void,
    NetworkIdentifier const&        source,
    PlayerAuthenticationInfo const& playerInfo
) {
    origin(source, playerInfo);

    if (!isDisconnect(source)) {
        getLLEventBus().publish(ClientLoginedEvent{source, const_cast<PlayerAuthenticationInfo&>(playerInfo)});
    }
}

EventHook(ClientLoginingEvent, ClientLoginedEvent, <ClientLoginEventHook1, ClientLoginEventHook2>);

} // namespace ila::server