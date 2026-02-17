#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>

// clang-format off
class BlockPos;
// clang-format on

namespace ila::mc::inline block
{
class SculkBlockGrowthBeforeEvent final : public ll::event::Cancellable<ll::event::WorldEvent>
{
public:
    BlockPos& mPos;

public:
    constexpr explicit SculkBlockGrowthBeforeEvent(BlockSource& blockSource, BlockPos& pos)
        : Cancellable(blockSource)
        , mPos(pos)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class SculkBlockGrowthAfterEvent final : public ll::event::WorldEvent
{
public:
    BlockPos const& mPos;

public:
    constexpr explicit SculkBlockGrowthAfterEvent(BlockSource& blockSource, BlockPos const& pos)
        : WorldEvent(blockSource)
        , mPos(pos)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline block
