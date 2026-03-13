#include "ila/event/explosion/ExplosionCollisionOffsetEvent.h"
#include "ila/base/Gloabl.i.h"

namespace ila::mc::inline explosion {

void ExplosionCollisionOffsetEvent::serialize(CompoundTag& nbt) const {
    ExplosionEvent::serialize(nbt);
    reflection::serialize_to(nbt["origin_pos"], mOriginPos).value();
    reflection::serialize_to(nbt["offset_pos"], mOffsetPos).value();
}

void ExplosionCollisionOffsetEvent::deserialize(CompoundTag const& nbt) {
    ExplosionEvent::deserialize(nbt);
    reflection::deserialize(mOffsetPos, nbt["offset_pos"]).value();
}

} // namespace ila::mc::inline explosion