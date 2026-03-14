#include "ila/event/packet/PacketEvent.h"
#include <ll/api/service/Bedrock.h>
#include <mc/client/game/ClientInstance.h>
#include <mc/client/player/LocalPlayer.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/server/ServerPlayer.h>

namespace ila::packet {

optional_ref<Player> PacketEvent::player() const {
    // clang-format off
    return isServerSide()
        ? ll::service::getClientInstance()
            .transform([](auto&& client) -> Player* {
                return client.getLocalPlayer();
            })
        : ll::service::getServerNetworkHandler(false)
            .transform([this](auto&& network) -> Player* {
                return network._getServerPlayer(mNetworkIdentifier, mSenderSubId);
            });
    // clang-format on
}

} // namespace ila::packet