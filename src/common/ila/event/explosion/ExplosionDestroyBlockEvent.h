#pragma once
#include "ila/event/explosion/ExplosionProcessBlockEvent.h"

namespace ila::mc::inline explosion {

/** @warning This event is not available on the client side. */
class ExplosionDestroyBlockingEvent final : public ll::event::Cancellable<ExplosionProcessBlockEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class ExplosionDestroyBlockedEvent final : public ExplosionProcessBlockEvent {
public:
    using ExplosionProcessBlockEvent::ExplosionProcessBlockEvent;
};

} // namespace ila::mc::inline explosion