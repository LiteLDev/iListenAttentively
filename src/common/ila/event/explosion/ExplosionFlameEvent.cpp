#include "ila/event/explosion/ExplosionFlameEvent.h"
#include "ExplosionEvent.h"
#include "ila/base/Gloabl.i.h"
#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>
#include <mc/nbt/CompoundTag.h>

namespace ila::explosion {

void ExplosionFlameEvent::serialize(CompoundTag& nbt) const {
    ExplosionEvent::serialize(nbt);
    reflection::serialize_to(nbt["pos"], mPos).value();
}

void ExplosionFlameEvent::deserialize(CompoundTag const& nbt) {
    ExplosionEvent::deserialize(nbt);
    reflection::deserialize(mPos, nbt["pos"]).value();
}

} // namespace ila::explosion