#pragma once
#include "ila/event/explosion/ExplosionEvent.h"
#include <ila/base/Macro.h>
#include <ll/api/event/Cancellable.h>
#include <mc/deps/nbt/CompoundTag.h>
#include <mc/world/level/Explosion.h>
#include <mc/world/level/IBlockSource.h>

namespace ila::explosion {

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

} // namespace ila::explosion