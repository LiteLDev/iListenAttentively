#pragma once
#include "ila/event/packet/PacketEvent.h"
#include <ll/api/event/Cancellable.h>

namespace ila::packet {

class ReceivingPacketEvent final : public ll::event::Cancellable<PacketEvent> {
public:
    using Cancellable::Cancellable;
};

class ReceivedPacketEvent final : public PacketEvent {
public:
    using PacketEvent::PacketEvent;
};

} // namespace ila::packet