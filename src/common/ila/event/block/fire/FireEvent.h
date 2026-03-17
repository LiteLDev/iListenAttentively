#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/world/WorldEvent.h>
#include <mc/world/level/BlockPos.h>

namespace ila::block::inline fire {

class FireEvent : public ll::event::WorldEvent {
private:
    BlockPos const& mPos;

public:
    constexpr explicit FireEvent(BlockSource& region, BlockPos const& pos) : WorldEvent(region), mPos(pos) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;

public:
    BlockPos const& pos() const { return mPos; }
};

} // namespace ila::block::inline fire