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
class DragonEggBlockTeleportBeforeEvent final : public ll::event::Cancellable<ll::event::WorldEvent>
{
public:
    BlockPos const& mPos;
    Random&         mRandom;
    BlockPos&       mTargetPos;

public:
    constexpr explicit DragonEggBlockTeleportBeforeEvent(
        BlockSource&    blockSource,
        BlockPos const& pos,
        Random&         random,
        BlockPos&       targetPos
    )
        : Cancellable(blockSource)
        , mPos(pos)
        , mRandom(random)
        , mTargetPos(targetPos)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class DragonEggBlockTeleportAfterEvent final : public ll::event::WorldEvent
{
public:
    BlockPos const& mPos;
    Random const&   mRandom;
    BlockPos const& mTargetPos;

public:
    constexpr explicit DragonEggBlockTeleportAfterEvent(
        BlockSource&    blockSource,
        BlockPos const& pos,
        Random const&   random,
        BlockPos const& targetPos
    )
        : WorldEvent(blockSource)
        , mPos(pos)
        , mRandom(random)
        , mTargetPos(targetPos)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline block