#pragma once
#include "ila/base/Gloabl.i.h"
#include "ila/event/packet/ReceivePacketEvent.h"
#include <mc/network/NetEventCallback.h>

namespace ila::packet {

IncomingPacketFilterResult handleReceive(
    NetEventCallback&                 self, //
    NetworkIdentifierWithSubId const& id,
    bool                              isServerSide
);

} // namespace ila::packet