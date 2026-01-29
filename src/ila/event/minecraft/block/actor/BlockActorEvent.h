#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Event.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/level/block/actor/BlockActor.h>

namespace ila::mc::inline block::inline actor
{

class BlockActorEvent : public ll::event::Event
{
public:
    BlockActor& mSelf;

public:
    constexpr explicit BlockActorEvent(BlockActor& blockActor)
        : mSelf(blockActor)
    {
    }

    ILAPI void serialize(CompoundTag&) const override;

};

} // namespace ila::mc::inline block::inline actor