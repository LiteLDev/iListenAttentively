#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/IBlockSource.h>

// clang-format off
class BlockPos;
// clang-format on

namespace ila::mc::inline block
{
class FarmDecayBeforeEvent final : public ll::event::Cancellable<ll::event::WorldEvent>
{
public:
    BlockPos& mPos;
    Actor*&   mActor;
    float&    mFallDistance;

public:
    constexpr explicit FarmDecayBeforeEvent(
        BlockSource& blockSource,
        BlockPos&    pos,
        Actor*&      actor,
        float&       fallDistance
    )
        : Cancellable(blockSource)
        , mPos(pos)
        , mActor(actor)
        , mFallDistance(fallDistance)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class FarmDecayAfterEvent final : public ll::event::WorldEvent
{
public:
    BlockPos const& mPos;
    Actor* const&   mActor;
    float const&    mFallDistance;

public:
    constexpr explicit FarmDecayAfterEvent(
        BlockSource&    blockSource,
        BlockPos const& pos,
        Actor* const&   actor,
        float const&    fallDistance
    )
        : WorldEvent(blockSource)
        , mPos(pos)
        , mActor(actor)
        , mFallDistance(fallDistance)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline block