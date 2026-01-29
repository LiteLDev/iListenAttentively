#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/entity/MobEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/Mob.h>
#include <mc/world/level/block/Block.h>

// clang-format off
class BlockPos;
// clang-format on

namespace ila::mc::inline actor::inline mob
{
class MobPlaceBlockBeforeEvent final : public ll::event::Cancellable<ll::event::entity::MobEvent>
{
public:
    BlockPos&    mPos;
    Block const* mBlock;

public:
    constexpr explicit MobPlaceBlockBeforeEvent(Mob& mob, BlockPos& pos, Block const* block)
        : Cancellable(mob)
        , mPos(pos)
        , mBlock(block)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class MobPlaceBlockAfterEvent final : public ll::event::entity::MobEvent
{
public:
    BlockPos&    mPos;
    Block const* mBlock;

public:
    constexpr explicit MobPlaceBlockAfterEvent(Mob& mob, BlockPos& pos, Block const* block)
        : MobEvent(mob)
        , mPos(pos)
        , mBlock(block)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline actor::inline mob