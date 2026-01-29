#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/entity/MobEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/Mob.h>

// clang-format off
class BlockPos;
// clang-format on

namespace ila::mc::inline actor::inline mob
{
class MobTakeBlockBeforeEvent final : public ll::event::Cancellable<ll::event::entity::MobEvent>
{
public:
    BlockPos& mPos;

public:
    constexpr explicit MobTakeBlockBeforeEvent(Mob& mob, BlockPos& pos)
        : Cancellable(mob)
        , mPos(pos)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class MobTakeBlockAfterEvent final : public ll::event::entity::MobEvent
{
public:
    BlockPos& mPos;

public:
    constexpr explicit MobTakeBlockAfterEvent(Mob& mob, BlockPos& pos)
        : MobEvent(mob)
        , mPos(pos)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline actor::inline mob