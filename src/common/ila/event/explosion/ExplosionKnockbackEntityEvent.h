#pragma once
#include "ila/event/explosion/ExplosionProcessEntityEvent.h"

namespace ila::explosion {

class ExplosionKnockbackEntityEvent : public ExplosionProcessEntityEvent {
protected:
    Vec3& mKnockback;

public:
    constexpr explicit ExplosionKnockbackEntityEvent(Explosion& explosion, Actor& entity, Vec3& knockback)
    : ExplosionProcessEntityEvent(explosion, entity),
      mKnockback(knockback) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

public:
    Vec3& knockback() const { return mKnockback; }
};

/** @warning This event is not available on the client side. */
class ExplosionKnockbackEntityingEvent final : public ll::event::Cancellable<ExplosionKnockbackEntityEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class ExplosionKnockbackEntityedEvent final : public ExplosionKnockbackEntityEvent {
public:
    using ExplosionKnockbackEntityEvent::ExplosionKnockbackEntityEvent;
};

} // namespace ila::explosion