#include "ila/event/explosion/ExplosionParticleEvent.h"
#include "ExplosionEvent.h"
#include "ila/base/Gloabl.i.h"
#include "patch_mc/deps/shared_types/legacy/LevelEvent.i.h"
#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>
#include <mc/nbt/CompoundTag.h>

namespace ila::explosion {

void ExplosionParticleEvent::serialize(CompoundTag& nbt) const {
    ExplosionEvent::serialize(nbt);
    reflection::serialize_to(nbt["pos"], mPos).value();
    reflection::serialize_to(nbt["particle"], mParticle).value();
}

void ExplosionParticleEvent::deserialize(CompoundTag const& nbt) {
    ExplosionEvent::deserialize(nbt);
    reflection::deserialize(mPos, nbt["pos"]).value();
}

} // namespace ila::explosion