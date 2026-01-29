#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>

// clang-format off
class Level;
class AABB;
// clang-format on

namespace ila::mc::inline actor::inline boss
{
class WitherDestroyBeforeEvent final : public ll::event::Cancellable<ll::event::WorldEvent>
{
public:
    Level& mLevel;
    AABB&  mBox;
    int&   mRadius;

public:
    constexpr explicit WitherDestroyBeforeEvent(
        BlockSource& blockSource,
        Level&       level,
        AABB&        box,
        int&         radius
    )
        : Cancellable(blockSource)
        , mLevel(level)
        , mBox(box)
        , mRadius(radius)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class WitherDestroyAfterEvent final : public ll::event::WorldEvent
{
public:
    Level&      mLevel;
    AABB const& mBox;
    int const&  mRadius;

public:
    constexpr explicit WitherDestroyAfterEvent(
        BlockSource& blockSource,
        Level&       level,
        AABB const&  box,
        int const&   radius
    )
        : WorldEvent(blockSource)
        , mLevel(level)
        , mBox(box)
        , mRadius(radius)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline actor::inline boss
