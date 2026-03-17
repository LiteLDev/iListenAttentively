#include "ila/event/explosion/ExplosionProcessEntityEvent.h"
#include "ExplosionEvent.h"
#include "ila/base/Gloabl.i.h"
#include <ll/api/event/EventRefObjSerializer.h>
#include <mc/nbt/CompoundTag.h>

namespace ila::explosion {

void ExplosionProcessEntityEvent::serialize(CompoundTag& nbt) const {
    ExplosionEvent::serialize(nbt);
    nbt["entity"] = serializeRefObj(mEntity);
}

} // namespace ila::explosion