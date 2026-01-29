#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/block/actor/ChestBlockActor.h>

// clang-format off
class BlockPos;
// clang-format on

namespace ila::mc::inline block::inline actor
{
class ChestPairWithBeforeEvent final : public ll::event::Cancellable<ll::event::WorldEvent>
{
public:
    ChestBlockActor& mChest;
    BlockPos&        mPosition;

public:
    constexpr explicit ChestPairWithBeforeEvent(
        BlockSource&     blockSource,
        ChestBlockActor& chest,
        BlockPos&        position
    )
        : Cancellable(blockSource)
        , mChest(chest)
        , mPosition(position)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class ChestPairWithAfterEvent final : public ll::event::WorldEvent
{
public:
    ChestBlockActor& mChest;
    BlockPos const&  mPosition;

public:
    constexpr explicit ChestPairWithAfterEvent(
        BlockSource&     blockSource,
        ChestBlockActor& chest,
        BlockPos const&  position
    )
        : WorldEvent(blockSource)
        , mChest(chest)
        , mPosition(position)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline block::inline actor