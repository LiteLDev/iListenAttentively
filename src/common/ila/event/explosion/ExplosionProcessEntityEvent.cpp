#include "ila/event/explosion/ExplosionProcessEntityEvent.h"
#include "ila/base/Gloabl.i.h"

namespace ila::explosion {

void ExplosionProcessEntityEvent::serialize(CompoundTag& nbt) const {
    ExplosionEvent::serialize(nbt);
    nbt["entity"] = serializeRefObj(mEntity);
}

} // namespace ila::explosion