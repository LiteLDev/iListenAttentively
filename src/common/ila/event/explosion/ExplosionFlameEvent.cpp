#include "ila/event/explosion/ExplosionFlameEvent.h"
#include "ila/base/Gloabl.i.h"

namespace ila::mc::inline explosion {

void ExplosionFlameEvent::serialize(CompoundTag& nbt) const {
    ExplosionEvent::serialize(nbt);
    reflection::serialize_to(nbt["pos"], mPos).value();
}

void ExplosionFlameEvent::deserialize(CompoundTag const& nbt) {
    ExplosionEvent::deserialize(nbt);
    reflection::deserialize(mPos, nbt["pos"]).value();
}

} // namespace ila::mc::inline explosion