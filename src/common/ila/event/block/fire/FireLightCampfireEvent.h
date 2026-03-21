#pragma once
#include "ila/event/block/fire/FireEvent.h"
#include <ll/api/event/Cancellable.h>

namespace ila::block::inline fire {

class FireLightCampfireEvent : public FireEvent {
private:
    BlockPos const& mTNTPos;

public:
    constexpr explicit FireLightCampfireEvent(BlockSource& region, BlockPos const& pos, BlockPos const& campfirePos)
    : FireEvent(region, pos),
      mTNTPos(campfirePos) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;

public:
    BlockPos const& tntPos() const { return mTNTPos; }
};

class FireLightingCampfireEvent final : public ll::event::Cancellable<FireLightCampfireEvent> {
public:
    using Cancellable::Cancellable;
};

class FireLightedCampfireEvent final : public FireLightCampfireEvent {
public:
    using FireLightCampfireEvent::FireLightCampfireEvent;
};

} // namespace ila::block::inline fire