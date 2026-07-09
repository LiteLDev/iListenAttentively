#pragma once
#include "ila/event/explosion/ExplosionEvent.h"
#include <ila/base/Macro.h>
#include <ll/api/event/Cancellable.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/nbt/CompoundTag.h>
#include <mc/world/level/Explosion.h>

namespace ila::explosion {

class ExplosionCollisionOffsetEvent : public ExplosionEvent {
protected:
    Vec3 const& mOriginPos;
    Vec3&       mOffsetPos;

public:
    constexpr explicit ExplosionCollisionOffsetEvent(Explosion& explosion, Vec3 const& originPos, Vec3& offsetPos)
    : ExplosionEvent(explosion),
      mOriginPos(originPos),
      mOffsetPos(offsetPos) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

public:
    Vec3 const& originPos() const { return mOriginPos; }
    Vec3&       offsetPos() const { return mOffsetPos; }
};

/** @warning This event is not available on the client side. */
class ExplosionCollidingEvent final : public ll::event::Cancellable<ExplosionCollisionOffsetEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class ExplosionCollidedEvent final : public ExplosionCollisionOffsetEvent {
public:
    using ExplosionCollisionOffsetEvent::ExplosionCollisionOffsetEvent;
};

} // namespace ila::explosion