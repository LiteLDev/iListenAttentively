#include "ila/event/explosion/ExplosionExperienceBlockEvent.h"
#include "ExplosionProcessBlockEvent.h"
#include "ila/base/Gloabl.i.h"
#include <mc/deps/nbt/CompoundTag.h>

namespace ila::explosion {

void ExplosionExperienceBlockEvent::serialize(CompoundTag& nbt) const {
    ExplosionProcessBlockEvent::serialize(nbt);
    nbt["experience"] = mExperience;
}

void ExplosionExperienceBlockEvent::deserialize(CompoundTag const& nbt) {
    ExplosionProcessBlockEvent::deserialize(nbt);
    mExperience = nbt["experience"];
}

} // namespace ila::explosion