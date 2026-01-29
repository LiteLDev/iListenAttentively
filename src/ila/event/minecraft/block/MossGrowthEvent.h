#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/level/BlockSource.h>
#include <vector>

// clang-format off
class BlockPos;
class Random;
// clang-format on

namespace ila::mc::inline block
{
class MossGrowthBeforeEvent final : public ll::event::Cancellable<ll::event::WorldEvent>
{
public:
    BlockPos& mPos;
    Random&   mRandom;
    int&      mXRadius;
    int&      mZRadius;

public:
    constexpr explicit MossGrowthBeforeEvent(
        BlockSource& blockSource,
        BlockPos&    pos,
        Random&      random,
        int&         xRadius,
        int&         zRadius
    )
        : Cancellable(blockSource)
        , mPos(pos)
        , mRandom(random)
        , mXRadius(xRadius)
        , mZRadius(zRadius)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class MossGrowthAfterEvent final : public ll::event::WorldEvent
{
public:
    BlockPos const&        mPos;
    Random const&          mRandom;
    int const&             mXRadius;
    int const&             mZRadius;
    std::vector<BlockPos>& mTargetPoss;

public:
    constexpr explicit MossGrowthAfterEvent(
        BlockSource&           blockSource,
        BlockPos const&        pos,
        Random const&          random,
        int const&             xRadius,
        int const&             zRadius,
        std::vector<BlockPos>& targetPoss
    )
        : WorldEvent(blockSource)
        , mPos(pos)
        , mRandom(random)
        , mXRadius(xRadius)
        , mZRadius(zRadius)
        , mTargetPoss(targetPoss)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};
} // namespace ila::mc::inline block