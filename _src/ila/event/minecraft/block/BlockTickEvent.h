#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/level/BlockSource.h>

// clang-format off
class Random;
class BlockPos;
// clang-format on

namespace ila::mc::inline block
{
class BlockTickBeforeEvent final : public ll::event::Cancellable<ll::event::WorldEvent>
{
public:
    BlockPos& mPos;
    Random&   mRandom;

public:
    constexpr explicit BlockTickBeforeEvent(BlockSource& blockSource, BlockPos& pos, Random& random)
        : Cancellable(blockSource)
        , mPos(pos)
        , mRandom(random)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class BlockTickAfterEvent final : public ll::event::WorldEvent
{
public:
    BlockPos const& mPos;
    Random const&   mRandom;

public:
    constexpr explicit BlockTickAfterEvent(
        BlockSource&    blockSource,
        BlockPos const& pos,
        Random const&   random
    )
        : WorldEvent(blockSource)
        , mPos(pos)
        , mRandom(random)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline block