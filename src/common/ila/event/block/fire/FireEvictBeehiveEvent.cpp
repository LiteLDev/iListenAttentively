#include "ila/event/block/fire/FireEvictBeehiveEvent.h"
#include "ila/base/Gloabl.i.h"
#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>
#include <mc/nbt/CompoundTag.h>

namespace ila::block::inline fire {

void FireEvictBeehiveEvent::serialize(CompoundTag& nbt) const {
    FireEvent::serialize(nbt);
    reflection::serialize_to(nbt["beehive_pos"], mBeehivePos).value();
}

} // namespace ila::block::inline fire