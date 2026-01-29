#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>

// clang-format off
class BlockPos;
// clang-format on

namespace ila::mc::inline block
{
class FireTryBurnBlockBeforeEvent final : public ll::event::Cancellable<ll::event::WorldEvent>
{
public:
    BlockPos const& mPos;

public:
    constexpr explicit FireTryBurnBlockBeforeEvent(BlockSource& blockSource, BlockPos const& pos)
        : Cancellable(blockSource)
        , mPos(pos)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};

class FireTryBurnBlockAfterEvent final : public ll::event::WorldEvent
{
public:
    BlockPos const& mPos;

public:
    constexpr explicit FireTryBurnBlockAfterEvent(BlockSource& blockSource, BlockPos const& pos)
        : WorldEvent(blockSource)
        , mPos(pos)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline block
