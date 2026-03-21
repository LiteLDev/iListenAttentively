#include "ila/event/block/fire/FireSpreadEvent.h"
#include "ila/base/Gloabl.i.h"
#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>
#include <mc/nbt/CompoundTag.h>

namespace ila::block::inline fire {

void FireSpreadEvent::serialize(CompoundTag& nbt) const {
    FireEvent::serialize(nbt);
    reflection::serialize_to(nbt["spread_pos"], mSpreadPos).value();
}

void FireSpreadEvent::deserialize(CompoundTag const& nbt) {
    FireEvent::deserialize(nbt);
    reflection::deserialize(mSpreadPos, nbt["spread_pos"]).value();
}

} // namespace ila::block::inline fire