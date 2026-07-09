#include "ila/event/explosion/ExplosionKnockbackEntityEvent.h"
#include "ExplosionProcessEntityEvent.h"
#include "ila/base/Gloabl.i.h"
#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>
#include <mc/deps/nbt/CompoundTag.h>

namespace ila::explosion {

void ExplosionKnockbackEntityEvent::serialize(CompoundTag& nbt) const {
    ExplosionProcessEntityEvent::serialize(nbt);
    reflection::serialize_to(nbt["knockback"], mKnockback).value();
}

void ExplosionKnockbackEntityEvent::deserialize(CompoundTag const& nbt) {
    ExplosionProcessEntityEvent::deserialize(nbt);
    reflection::deserialize(mKnockback, nbt["knockback"]).value();
}

} // namespace ila::explosion