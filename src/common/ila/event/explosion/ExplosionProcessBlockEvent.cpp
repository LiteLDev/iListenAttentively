#include "ila/event/explosion/ExplosionProcessBlockEvent.h"
#include "ExplosionEvent.h"
#include "ila/base/Gloabl.i.h"
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/reflection/Serialization.h>
#include <mc/nbt/CompoundTag.h>

namespace ila::explosion {

void ExplosionProcessBlockEvent::serialize(CompoundTag& nbt) const {
    ExplosionEvent::serialize(nbt);
    ll::reflection::serialize_to(nbt["pos"], mPos).value();
    nbt["block"]          = serializeRefObj(mBlock);
    nbt["is_extra_block"] = mIsExtraBlock;
}

} // namespace ila::explosion