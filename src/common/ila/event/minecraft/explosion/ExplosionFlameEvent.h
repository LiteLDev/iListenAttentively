#pragma once
#include "ila/event/minecraft/explosion/ExplosionEvent.h"

namespace ila::mc::inline explosion {

class ExplosionFlameEvent : public ExplosionEvent {
protected:
    BlockPos& mPos;

public:
    constexpr explicit ExplosionFlameEvent(Explosion& explosion, BlockPos& pos)
    : ExplosionEvent(explosion),
      mPos(pos) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

public:
    BlockPos& pos() const { return mPos; }
};

/** @warning This event is not available on the client side. */
class ExplosionFlamingEvent final : public ll::event::Cancellable<ExplosionFlameEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class ExplosionFlamedEvent final : public ExplosionFlameEvent {
public:
    using ExplosionFlameEvent::ExplosionFlameEvent;
};

} // namespace ila::mc::inline explosion