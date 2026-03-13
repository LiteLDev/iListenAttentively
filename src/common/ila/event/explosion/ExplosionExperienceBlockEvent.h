#pragma once
#include "ila/event/explosion/ExplosionProcessBlockEvent.h"
#include <mc/world/actor/ActorDamageSource.h>

namespace ila::mc::inline explosion {

class ExplosionExperienceBlockEvent : public ExplosionProcessBlockEvent {
protected:
    int& mExperience;

public:
    constexpr explicit ExplosionExperienceBlockEvent(
        Explosion&      explosion,
        BlockPos const& pos,
        Block const&    block,
        bool            isExtraBlock,
        int&            experience
    )
    : ExplosionProcessBlockEvent(explosion, pos, block, isExtraBlock),
      mExperience(experience) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

public:
    int& experience() { return mExperience; }
};

/** @warning This event is not available on the client side. */
class ExplosionExperienceBlockingEvent final : public ll::event::Cancellable<ExplosionExperienceBlockEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class ExplosionExperienceBlockedEvent final : public ExplosionExperienceBlockEvent {
public:
    using ExplosionExperienceBlockEvent::ExplosionExperienceBlockEvent;
};

} // namespace ila::mc::inline explosion