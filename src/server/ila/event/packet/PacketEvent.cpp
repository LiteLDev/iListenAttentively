#include "ila/event/packet/PacketEvent.h"
#include <ll/api/service/Bedrock.h>
#include <mc/deps/core/utility/optional_ref.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/server/ServerPlayer.h>
#include <mc/world/actor/player/Player.h>

namespace ila::packet {

optional_ref<Player> PacketEvent::player() const {
    return ll::service::getServerNetworkHandler()->_getServerPlayer(mNetworkIdentifier, mSenderSubId);
}

} // namespace ila::packet