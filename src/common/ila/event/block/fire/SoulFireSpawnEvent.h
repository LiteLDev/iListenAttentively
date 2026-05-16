#pragma once
#include "ila/event/block/fire/FireEvent.h"
#include <ll/api/event/Cancellable.h>

namespace ila::block::inline fire {

/** @warning This event is not available on the client side. */
class SoulFireSpawningEvent final : public ll::event::Cancellable<FireEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class SoulFireSpawnedEvent final : public FireEvent {
public:
    using FireEvent::FireEvent;
};

} // namespace ila::block::inline fire