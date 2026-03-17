#pragma once
#include "ila/event/block/fire/FireEvent.h"
#include <ila/base/Macro.h>
#include <ll/api/event/Cancellable.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>

namespace ila::block::inline fire {

class FireSpreadEvent : public FireEvent {
private:
    BlockPos& mSpreadPos;

public:
    constexpr explicit FireSpreadEvent(BlockSource& region, BlockPos const& pos, BlockPos& spreadPos)
    : FireEvent(region, pos),
      mSpreadPos(spreadPos) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

public:
    BlockPos& spreadPos() { return mSpreadPos; }
};

class FireSpreadingEvent final : public ll::event::Cancellable<FireSpreadEvent> {
public:
    using Cancellable::Cancellable;
};

class FireSpreadedEvent final : public FireSpreadEvent {
public:
    using FireSpreadEvent::FireSpreadEvent;
};

} // namespace ila::block::inline fire