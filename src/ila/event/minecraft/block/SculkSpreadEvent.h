#pragma once
#include "ila/base/Macro.h"
#include <ll/api/base/StdInt.h>
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/level/BlockSource.h>

// clang-format off
class Block;
class BlockPos;
// clang-format on

namespace ila::mc::inline block
{
class SculkSpreadBeforeEvent final : public ll::event::Cancellable<ll::event::WorldEvent>
{
public:
    BlockPos& mSelfPos;
    Block&    mSelfBlock;
    uchar&    mSelfFace;
    BlockPos& mTargetPos;
    Block&    mTargetBlock;
    uchar&    mTargetFace;
    uchar&    mFacing;

public:
    constexpr explicit SculkSpreadBeforeEvent(
        BlockSource& blockSource,
        BlockPos&    selfPos,
        Block&       selfBlock,
        uchar&       selfFace,
        BlockPos&    targetPos,
        Block&       targetBlock,
        uchar&       targetFace,
        uchar&       facing
    )
        : Cancellable(blockSource)
        , mSelfPos(selfPos)
        , mSelfBlock(selfBlock)
        , mSelfFace(selfFace)
        , mTargetPos(targetPos)
        , mTargetBlock(targetBlock)
        , mTargetFace(targetFace)
        , mFacing(facing)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class SculkSpreadAfterEvent final : public ll::event::WorldEvent
{
public:
    BlockPos const& mSelfPos;
    Block const&    mSelfBlock;
    uchar const&    mSelfFace;
    BlockPos const& mTargetPos;
    Block const&    mTargetBlock;
    uchar const&    mTargetFace;
    uchar const&    mFacing;

public:
    constexpr explicit SculkSpreadAfterEvent(
        BlockSource&    blockSource,
        BlockPos const& selfPos,
        Block const&    selfBlock,
        uchar const&    selfFace,
        BlockPos const& targetPos,
        Block const&    targetBlock,
        uchar const&    targetFace,
        uchar const&    facing
    )
        : WorldEvent(blockSource)
        , mSelfPos(selfPos)
        , mSelfBlock(selfBlock)
        , mSelfFace(selfFace)
        , mTargetPos(targetPos)
        , mTargetBlock(targetBlock)
        , mTargetFace(targetFace)
        , mFacing(facing)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline block