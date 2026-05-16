#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>

namespace ila::world {

class CircuitSystemConnectionEvent : public ll::event::WorldEvent {
private:
    BlockPos const& mFromPos;
    BlockPos const& mToPos;

public:
    constexpr explicit CircuitSystemConnectionEvent(BlockSource& region, BlockPos const& fromPos, BlockPos const& toPos)
    : WorldEvent(region),
      mFromPos(fromPos),
      mToPos(toPos) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;

public:
    BlockPos const& fromPos() const { return mFromPos; }
    BlockPos const& toPos() const { return mToPos; }
};

/** @warning This event is not available on the client side. */
class CircuitSystemConnectingEvent final : public ll::event::Cancellable<CircuitSystemConnectionEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class CircuitSystemConnectedEvent final : public CircuitSystemConnectionEvent {
public:
    using CircuitSystemConnectionEvent::CircuitSystemConnectionEvent;
};

} // namespace ila::world