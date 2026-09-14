#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>
#include <mc/world/actor/boss/WitherBoss.h>

// clang-format off
class Level;
class AABB;
// clang-format on

namespace ila::mc::inline world {
class WitherDestroyBeforeEvent final : public ll::event::Cancellable<ll::event::WorldEvent> {
protected:
    Level&                        mLevel;
    AABB&                         mBox;
    int&                          mRadius;
    WitherBoss::WitherAttackType& mType;
    WitherBoss&                   mWither;

public:
    constexpr explicit WitherDestroyBeforeEvent(
        BlockSource&                  blockSource,
        Level&                        level,
        AABB&                         box,
        int&                          radius,
        WitherBoss::WitherAttackType& type,
        WitherBoss&                   wither
    )
    : Cancellable(blockSource),
      mLevel(level),
      mBox(box),
      mRadius(radius),
      mType(type),
      mWither(wither) {}

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

    ILNDAPI Level& level() const;
    ILNDAPI AABB&  box() const;
    ILNDAPI int&   radius() const;
    ILNDAPI WitherBoss::WitherAttackType& type() const;
    ILNDAPI WitherBoss&                   wither() const;
};

class WitherDestroyAfterEvent final : public ll::event::WorldEvent {
protected:
    Level&                              mLevel;
    AABB const&                         mBox;
    int const&                          mRadius;
    WitherBoss::WitherAttackType const& mType;
    WitherBoss&                         mWither;

public:
    constexpr explicit WitherDestroyAfterEvent(
        BlockSource&                        blockSource,
        Level&                              level,
        AABB const&                         box,
        int const&                          radius,
        WitherBoss::WitherAttackType const& type,
        WitherBoss&                         wither
    )
    : WorldEvent(blockSource),
      mLevel(level),
      mBox(box),
      mRadius(radius),
      mType(type),
      mWither(wither) {}

    ILAPI void serialize(CompoundTag& nbt) const override;

    ILNDAPI Level&      level() const;
    ILNDAPI AABB const& box() const;
    ILNDAPI int const&  radius() const;
    ILNDAPI WitherBoss::WitherAttackType const& type() const;
    ILNDAPI WitherBoss&                         wither() const;
};
} // namespace ila::mc::inline world
