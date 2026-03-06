#include "ila/event/minecraft/explosion/ExplosionKnockbackEntityEvent.h"
#include "ila/base/Gloabl.i.h"

namespace ila::mc::inline explosion {

void ExplosionKnockbackEntityEvent::serialize(CompoundTag& nbt) const {
    ExplosionProcessEntityEvent::serialize(nbt);
    reflection::serialize_to(nbt["knockback"], mKnockback).value();
}

void ExplosionKnockbackEntityEvent::deserialize(CompoundTag const& nbt) {
    ExplosionProcessEntityEvent::deserialize(nbt);
    reflection::deserialize(mKnockback, nbt["knockback"]).value();
}

} // namespace ila::mc::inline explosion