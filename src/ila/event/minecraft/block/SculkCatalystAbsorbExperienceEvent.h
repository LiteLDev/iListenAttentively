#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/LevelEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/level/IBlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/actor/SculkCatalystBlockActor.h>

namespace ila::mc::inline block
{
class SculkCatalystAbsorbExperienceBeforeEvent final : public ll::event::Cancellable<ll::event::LevelEvent>
{
public:
    SculkCatalystBlockActor& mBlockActor;
    Actor&                   mActor;

public:
    constexpr explicit SculkCatalystAbsorbExperienceBeforeEvent(
        Level&                   level,
        SculkCatalystBlockActor& blockActor,
        Actor&                   actor
    )
        : Cancellable(level)
        , mBlockActor(blockActor)
        , mActor(actor)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};

class SculkCatalystAbsorbExperienceAfterEvent final : public ll::event::LevelEvent
{
public:
    SculkCatalystBlockActor& mBlockActor;
    Actor&                   mActor;

public:
    constexpr explicit SculkCatalystAbsorbExperienceAfterEvent(
        Level&                   level,
        SculkCatalystBlockActor& blockActor,
        Actor&                   actor
    )
        : LevelEvent(level)
        , mBlockActor(blockActor)
        , mActor(actor)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline block