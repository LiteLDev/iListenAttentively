#pragma once
#include "ila/event/explosion/ExplosionProcessEntityEvent.h"
#include <ila/base/Macro.h>
#include <ll/api/event/Cancellable.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/ActorDamageSource.h>
#include <mc/world/level/Explosion.h>
#include <mc/world/level/IBlockSource.h>

namespace ila::explosion {

class ExplosionDamageEntityEvent : public ExplosionProcessEntityEvent {
protected:
    ActorDamageSource& mSource;
    float&             mDamage;
    bool&              mKnockback;
    bool&              mIgnite;

public:
    constexpr explicit ExplosionDamageEntityEvent(
        Explosion&         explosion,
        Actor&             entity,
        ActorDamageSource& source,
        float&             damage,
        bool&              knockback,
        bool&              ignite
    )
    : ExplosionProcessEntityEvent(explosion, entity),
      mSource(source),
      mDamage(damage),
      mKnockback(knockback),
      mIgnite(ignite) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

public:
    ActorDamageSource& source() const { return mSource; }
    float&             damage() const { return mDamage; }
    bool&              knockback() const { return mKnockback; }
    bool&              ignite() const { return mIgnite; }
};

/** @warning This event is not available on the client side. */
class ExplosionDamageEntityingEvent final : public ll::event::Cancellable<ExplosionDamageEntityEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class ExplosionDamageEntityedEvent final : public ExplosionDamageEntityEvent {
public:
    using ExplosionDamageEntityEvent::ExplosionDamageEntityEvent;
};

} // namespace ila::explosion