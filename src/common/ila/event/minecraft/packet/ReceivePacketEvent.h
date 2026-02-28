#pragma once
#include "ila/event/minecraft/packet/PacketEvent.h"
#include <ll/api/event/Cancellable.h>

namespace ila::mc::inline packet {

class ReceivingPacketEvent final : public ll::event::Cancellable<PacketEvent> {
public:
    using Cancellable::Cancellable;
};

class ReceivedPacketEvent final : public PacketEvent {
public:
    using PacketEvent::PacketEvent;
};

} // namespace ila::mc::inline packet