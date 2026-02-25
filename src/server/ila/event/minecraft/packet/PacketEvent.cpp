#include "ila/event/minecraft/packet/PacketEvent.h"
#include <ll/api/service/Bedrock.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/server/ServerPlayer.h>

namespace ila::mc::inline packet {

optional_ref<Player> PacketEvent::player() const {
    return ll::service::getServerNetworkHandler()->_getServerPlayer(mNetworkIdentifier, mSenderSubId);
}

} // namespace ila::mc::inline packet