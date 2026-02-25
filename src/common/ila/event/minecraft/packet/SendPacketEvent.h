#pragma once
#include "ila/event/minecraft/packet/PacketEvent.h"
#include <ll/api/event/Cancellable.h>

namespace ila::mc::inline packet {

class SendingPacketEvent final : public ll::event::Cancellable<PacketEvent> {
public:
    using Cancellable::Cancellable;
};

class SentPacketEvent final : public ll::event::Cancellable<PacketEvent> {
public:
    using Cancellable::Cancellable;
};

} // namespace ila::mc::inline packet