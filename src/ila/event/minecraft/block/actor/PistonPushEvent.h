#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>

// clang-format off
class BlockPos;
// clang-format on

namespace ila::mc::inline block::inline actor
{
class PistonPushBeforeEvent final : public ll::event::Cancellable<ll::event::WorldEvent>
{
public:
    BlockPos& mPistonPos;
    BlockPos& mPushPos;
    uchar&    mBranchFacing;
    uchar&    mPistonMoveFacing;

public:
    constexpr explicit PistonPushBeforeEvent(
        BlockSource& blockSource,
        BlockPos&    pistonPos,
        BlockPos&    pushPos,
        uchar&       branchFacing,
        uchar&       pistonMoveFacing
    )
        : Cancellable(blockSource)
        , mPistonPos(pistonPos)
        , mPushPos(pushPos)
        , mBranchFacing(branchFacing)
        , mPistonMoveFacing(pistonMoveFacing)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class PistonPushAfterEvent final : public ll::event::WorldEvent
{
public:
    BlockPos const& mPistonPos;
    BlockPos const& mPushPos;
    uchar const&    mBranchFacing;
    uchar const&    mPistonMoveFacing;

public:
    constexpr explicit PistonPushAfterEvent(
        BlockSource&    blockSource,
        BlockPos const& pistonPos,
        BlockPos const& pushPos,
        uchar&          branchFacing,
        uchar&          pistonMoveFacing
    )
        : WorldEvent(blockSource)
        , mPistonPos(pistonPos)
        , mPushPos(pushPos)
        , mBranchFacing(branchFacing)
        , mPistonMoveFacing(pistonMoveFacing)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline block::inline actor
