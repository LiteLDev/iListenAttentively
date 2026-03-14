#include "ila/event/packet/PacketEvent.h"
#include <ll/api/service/Bedrock.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/server/ServerPlayer.h>

namespace ila::packet {

optional_ref<Player> PacketEvent::player() const {
    return ll::service::getServerNetworkHandler()->_getServerPlayer(mNetworkIdentifier, mSenderSubId);
}

} // namespace ila::packet