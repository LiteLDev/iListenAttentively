#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>
#include <mc/deps/nbt/CompoundTag.h>
#include <mc/world/level/Explosion.h>

namespace ila::explosion {

class ExplosionEvent : public ll::event::WorldEvent {
private:
    Explosion& mExplosion;

public:
    constexpr explicit ExplosionEvent(Explosion& explosion) : WorldEvent(explosion.mRegion), mExplosion(explosion) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

public:
    Explosion& explosion() const { return mExplosion; }
};

/** @warning This event is not available on the client side. */
class ExplodingEvent final : public ll::event::Cancellable<ExplosionEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class ExplodedEvent final : public ExplosionEvent {
public:
    using ExplosionEvent::ExplosionEvent;
};

} // namespace ila::explosion