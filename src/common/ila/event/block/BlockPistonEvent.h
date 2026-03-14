#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/actor/PistonBlockActor.h>

namespace ila::block {

class BlockPistonEvent : public ll::event::WorldEvent {
private:
    PistonBlockActor& mPiston;
    Block const&      mPistonBlock;
    FacingID          mFacing;

public:
    BlockPistonEvent(BlockSource& source, PistonBlockActor& piston, Block const& pistonBlock, FacingID facing)
    : WorldEvent(source),
      mPiston(piston),
      mPistonBlock(pistonBlock),
      mFacing(facing) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;

public:
    PistonBlockActor& piston() const { return mPiston; }
    Block const&      pistonBlock() const { return mPistonBlock; }
    FacingID          facing() const { return mFacing; }
};

/** @warning This event is not available on the client side. */
class BlockPistonExtendEvent final : public ll::event::Cancellable<BlockPistonEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class BlockPistonRetractEvent final : public ll::event::Cancellable<BlockPistonEvent> {
public:
    using Cancellable::Cancellable;
};

} // namespace ila::block