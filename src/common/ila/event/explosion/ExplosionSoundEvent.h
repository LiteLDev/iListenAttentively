#pragma once
#include "ila/event/explosion/ExplosionEvent.h"

namespace ila::explosion {

class ExplosionSoundEvent : public ExplosionEvent {
public:
    using ExplosionEvent::ExplosionEvent;

public:
    SharedTypes::Legacy::LevelSoundEvent& sound() const { return explosion().mSoundExplosionType; }
};

/** @warning This event is not available on the client side. */
class ExplosionSoundingEvent final : public ll::event::Cancellable<ExplosionSoundEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class ExplosionSoundedEvent final : public ExplosionSoundEvent {
public:
    using ExplosionSoundEvent::ExplosionSoundEvent;
};

} // namespace ila::explosion