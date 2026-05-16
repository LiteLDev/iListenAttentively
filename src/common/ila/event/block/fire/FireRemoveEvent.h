#pragma once
#include "ila/event/block/fire/FireEvent.h"
#include <ll/api/event/Cancellable.h>

namespace ila::block::inline fire {

class FireRemoveEvent : public FireEvent {
public:
    enum class Reason {
        Unknown     = 0x0,
        Unsupported = 0x1,
        GameRule    = 0x2,
        Rain        = 0x3,
        Water       = 0x4,
        BurntOut    = 0x5,
    };

private:
    Reason mReason;

public:
    constexpr explicit FireRemoveEvent(BlockSource& region, BlockPos const& pos, Reason reason)
    : FireEvent(region, pos),
      mReason(reason) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;

public:
    Reason reason() const { return mReason; }
};

/** @warning This event is not available on the client side. */
class FireRemovingEvent final : public ll::event::Cancellable<FireRemoveEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class FireRemovedEvent final : public FireRemoveEvent {
public:
    using FireRemoveEvent::FireRemoveEvent;
};

} // namespace ila::block::inline fire