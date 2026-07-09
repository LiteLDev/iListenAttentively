#pragma once
#include "ila/event/block/fire/FireEvent.h"
#include <ila/base/Macro.h>
#include <ll/api/event/Cancellable.h>
#include <mc/deps/nbt/CompoundTag.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>

namespace ila::block::inline fire {

class FireAgeEvent : public FireEvent {
private:
    int  mPrevAge;
    int& mNewAge;

public:
    constexpr explicit FireAgeEvent(BlockSource& region, BlockPos const& pos, int prevAge, int& newAge)
    : FireEvent(region, pos),
      mPrevAge(prevAge),
      mNewAge(newAge) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

public:
    int  prevAge() const { return mPrevAge; }
    int& newAge() { return mNewAge; }
};

/** @warning This event is not available on the client side. */
class FireAgingEvent final : public ll::event::Cancellable<FireAgeEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class FireAgedEvent final : public FireAgeEvent {
public:
    using FireAgeEvent::FireAgeEvent;
};

} // namespace ila::block::inline fire