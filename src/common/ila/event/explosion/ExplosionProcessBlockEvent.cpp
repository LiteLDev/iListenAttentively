#include "ila/event/explosion/ExplosionProcessBlockEvent.h"
#include "ila/base/Gloabl.i.h"

namespace ila::mc::inline explosion {

void ExplosionProcessBlockEvent::serialize(CompoundTag& nbt) const {
    ExplosionEvent::serialize(nbt);
    ll::reflection::serialize_to(nbt["pos"], mPos).value();
    nbt["block"]          = serializeRefObj(mBlock);
    nbt["is_extra_block"] = mIsExtraBlock;
}

} // namespace ila::mc::inline explosion