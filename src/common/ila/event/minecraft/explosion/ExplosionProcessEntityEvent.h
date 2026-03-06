#pragma once
#include "ila/event/minecraft/explosion/ExplosionEvent.h"

namespace ila::mc::inline explosion {

class ExplosionProcessEntityEvent : public ExplosionEvent {
private:
    Actor& mEntity;

public:
    constexpr explicit ExplosionProcessEntityEvent(Explosion& explosion, Actor& entity)
    : ExplosionEvent(explosion),
      mEntity(entity) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;

public:
    Actor& entity() const { return mEntity; }
};

/** @warning This event is not available on the client side. */
class ExplosionProcessEntityingEvent final : public ll::event::Cancellable<ExplosionProcessEntityEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class ExplosionProcessEntityedEvent final : public ExplosionProcessEntityEvent {
public:
    using ExplosionProcessEntityEvent::ExplosionProcessEntityEvent;
};

} // namespace ila::mc::inline explosion