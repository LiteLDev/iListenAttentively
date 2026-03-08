#include "ila/event/minecraft/explosion/ExplosionParticleEvent.h"
#include "ila/base/Gloabl.i.h"
#include "patch_mc/deps/shared_types/legacy/LevelEvent.i.h"

namespace ila::mc::inline explosion {

void ExplosionParticleEvent::serialize(CompoundTag& nbt) const {
    ExplosionEvent::serialize(nbt);
    reflection::serialize_to(nbt["pos"], mPos).value();
    reflection::serialize_to(nbt["particle"], mParticle).value();
}

void ExplosionParticleEvent::deserialize(CompoundTag const& nbt) {
    ExplosionEvent::deserialize(nbt);
    reflection::deserialize(mPos, nbt["pos"]).value();
}

} // namespace ila::mc::inline explosion