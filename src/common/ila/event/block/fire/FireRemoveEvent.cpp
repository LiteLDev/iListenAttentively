#include "ila/event/block/fire/FireRemoveEvent.h"
#include "ila/base/Gloabl.i.h"

namespace ila::block::inline fire {

void FireRemoveEvent::serialize(CompoundTag& nbt) const {
    FireEvent::serialize(nbt);
    reflection::serialize_to(nbt["reason"], mReason).value();
}

} // namespace ila::block::inline fire