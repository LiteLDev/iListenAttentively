#pragma once
#include "ila/base/Gloabl.i.h"
#include "ila/event/packet/ReceivePacketEvent.h"
#include <mc/network/IncomingPacketFilterResult.h>
#include <mc/network/NetEventCallback.h>
#include <mc/network/NetworkIdentifierWithSubId.h>

namespace ila::packet {

IncomingPacketFilterResult handleReceive(
    NetEventCallback&                 self, //
    NetworkIdentifierWithSubId const& id,
    bool                              isServerSide
);

} // namespace ila::packet