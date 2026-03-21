#pragma once
#include "ila/event/block/fire/FireEvent.h"
#include <ll/api/event/Cancellable.h>

namespace ila::block::inline fire {

class FireBurnBlockEvent : public FireEvent {
private:
    BlockPos const& mBlockPos;

public:
    constexpr explicit FireBurnBlockEvent(BlockSource& region, BlockPos const& pos, BlockPos const& blockPos)
    : FireEvent(region, pos),
      mBlockPos(blockPos) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;

public:
    BlockPos const& blockPos() const { return mBlockPos; }
};

class FireBurningBlockEvent final : public ll::event::Cancellable<FireBurnBlockEvent> {
public:
    using Cancellable::Cancellable;
};

class FireBurnedBlockEvent final : public FireBurnBlockEvent {
public:
    using FireBurnBlockEvent::FireBurnBlockEvent;
};

} // namespace ila::block::inline fire