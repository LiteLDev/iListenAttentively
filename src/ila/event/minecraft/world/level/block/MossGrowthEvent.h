#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>

// clang-format off
class BlockPos;
class Random;
// clang-format on

namespace ila::mc::inline world::inline level::inline block
{
class MossGrowthBeforeEvent final : public ll::event::Cancellable<ll::event::WorldEvent>
{
protected:
    BlockPos& mPos;
    Random&   mRandom;

public:
    constexpr explicit MossGrowthBeforeEvent(BlockSource& blockSource, BlockPos& pos, Random& random)
        : Cancellable(blockSource)
        , mPos(pos)
        , mRandom(random)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

    ILNDAPI BlockPos& pos() const;
    ILNDAPI Random&   random() const;
};

class MossGrowthAfterEvent final : public ll::event::WorldEvent
{
protected:
    BlockPos const&          mPos;
    Random const&            mRandom;
    std::optional<BlockPos>& mTargetPos;

public:
    constexpr explicit MossGrowthAfterEvent(
        BlockSource&             blockSource,
        BlockPos const&          pos,
        Random const&            random,
        std::optional<BlockPos>& targetPos
    )
        : WorldEvent(blockSource)
        , mPos(pos)
        , mRandom(random)
        , mTargetPos(targetPos)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

    ILNDAPI BlockPos const& pos() const;
    ILNDAPI Random const&   random() const;
    ILNDAPI std::optional<BlockPos>& getTargetPos() const;
};
} // namespace ila::mc::inline world::inline level::inline block
