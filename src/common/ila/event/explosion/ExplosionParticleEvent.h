#pragma once
#include "ila/event/explosion/ExplosionEvent.h"

namespace ila::mc::inline explosion {

class ExplosionParticleEvent : public ExplosionEvent {
private:
    Vec3&                           mPos;
    SharedTypes::Legacy::LevelEvent mParticle;

public:
    constexpr explicit ExplosionParticleEvent(
        Explosion&                      explosion,
        Vec3&                           pos,
        SharedTypes::Legacy::LevelEvent particleType
    )
    : ExplosionEvent(explosion),
      mPos(pos),
      mParticle(particleType) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

public:
    Vec3&                           pos() const { return mPos; }
    SharedTypes::Legacy::LevelEvent particle() const { return mParticle; }
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