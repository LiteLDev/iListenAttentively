#include "ila/event/explosion/ExplosionExperienceBlockEvent.h"
#include "ila/base/Gloabl.i.h"

namespace ila::mc::inline explosion {

void ExplosionExperienceBlockEvent::serialize(CompoundTag& nbt) const {
    ExplosionProcessBlockEvent::serialize(nbt);
    nbt["experience"] = mExperience;
}

void ExplosionExperienceBlockEvent::deserialize(CompoundTag const& nbt) {
    ExplosionProcessBlockEvent::deserialize(nbt);
    mExperience = nbt["experience"];
}

} // namespace ila::mc::inline explosion