#pragma once
#include "ila/event/block/fire/FireEvent.h"
#include <ll/api/event/Cancellable.h>

namespace ila::block::inline fire {

class FireIgniteTNTEvent : public FireEvent {
private:
    BlockPos const& mTNTPos;

public:
    constexpr explicit FireIgniteTNTEvent(BlockSource& region, BlockPos const& pos, BlockPos const& tntPos)
    : FireEvent(region, pos),
      mTNTPos(tntPos) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;

public:
    BlockPos const& tntPos() const { return mTNTPos; }
};

/** @warning This event is not available on the client side. */
class FireIgnitingTNTEvent final : public ll::event::Cancellable<FireIgniteTNTEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class FireIgnitedTNTEvent final : public FireIgniteTNTEvent {
public:
    using FireIgniteTNTEvent::FireIgniteTNTEvent;
};

} // namespace ila::block::inline fire