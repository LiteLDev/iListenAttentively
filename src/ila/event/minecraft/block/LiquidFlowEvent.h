#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/level/BlockSource.h>

// clang-format off
class BlockPos;
// clang-format on

namespace ila::mc::inline block
{
class LiquidFlowBeforeEvent final : public ll::event::Cancellable<ll::event::WorldEvent>
{
public:
    BlockPos&       mPos;
    int&            mDepth;
    BlockPos const& mFlowFromPos;

public:
    constexpr explicit LiquidFlowBeforeEvent(
        BlockSource&    blockSource,
        BlockPos&       pos,
        int&            depth,
        BlockPos const& flowFromPos
    )
        : Cancellable(blockSource)
        , mPos(pos)
        , mDepth(depth)
        , mFlowFromPos(flowFromPos)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class LiquidFlowAfterEvent final : public ll::event::WorldEvent
{
public:
    BlockPos const& mPos;
    int const&      mDepth;
    BlockPos const& mFlowFromPos;

public:
    constexpr explicit LiquidFlowAfterEvent(
        BlockSource&    blockSource,
        BlockPos const& pos,
        int const&      depth,
        BlockPos const& flowFromPos
    )
        : WorldEvent(blockSource)
        , mPos(pos)
        , mDepth(depth)
        , mFlowFromPos(flowFromPos)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline block