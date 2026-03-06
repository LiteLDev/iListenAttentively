#pragma once
#include "ila/event/minecraft/explosion/ExplosionEvent.h"

namespace ila::mc::inline explosion {

class ExplosionParticleEvent : public ExplosionEvent {
public:
    using ExplosionEvent::ExplosionEvent;

public:
    SharedTypes::Legacy::LevelEvent& particle() const { return explosion().mParticleType; }
};

/** @warning This event is not available on the client side. */
class ExplosionParticlingEvent final : public ll::event::Cancellable<ExplosionParticleEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class ExplosionParticledEvent final : public ExplosionParticleEvent {
public:
    using ExplosionParticleEvent::ExplosionParticleEvent;
};

} // namespace ila::mc::inline explosion