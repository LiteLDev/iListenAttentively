#pragma once
#include "ila/event/explosion/ExplosionEvent.h"
#include <ila/base/Macro.h>
#include <ll/api/event/Cancellable.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/shared_types/legacy/LevelEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/level/Explosion.h>

namespace ila::explosion {

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

} // namespace ila::explosion