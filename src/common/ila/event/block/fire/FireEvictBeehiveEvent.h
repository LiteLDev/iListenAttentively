#pragma once
#include "ila/event/block/fire/FireEvent.h"
#include <ll/api/event/Cancellable.h>

namespace ila::block::inline fire {

class FireEvictBeehiveEvent : public FireEvent {
private:
    BlockPos const& mBeehivePos;

public:
    constexpr explicit FireEvictBeehiveEvent(BlockSource& region, BlockPos const& pos, BlockPos const& beehivePos)
    : FireEvent(region, pos),
      mBeehivePos(beehivePos) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;

public:
    BlockPos const& beehivePos() const { return mBeehivePos; }
};

/** @warning This event is not available on the client side. */
class FireEvictingBeehiveEvent final : public ll::event::Cancellable<FireEvictBeehiveEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class FireEvictedBeehiveEvent final : public FireEvictBeehiveEvent {
public:
    using FireEvictBeehiveEvent::FireEvictBeehiveEvent;
};

} // namespace ila::block::inline fire