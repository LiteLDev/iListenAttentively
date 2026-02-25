#pragma once
#include "ila/base/Gloabl.i.h"
#include "ila/event/minecraft/packet/ReceivePacketEvent.h"
#include <mc/network/NetEventCallback.h>

namespace ila::mc::inline packet {

IncomingPacketFilterResult handleReceive(
    NetEventCallback&                 self, //
    NetworkIdentifierWithSubId const& id,
    bool                              isServerSide
);

} // namespace ila::mc::inline packet