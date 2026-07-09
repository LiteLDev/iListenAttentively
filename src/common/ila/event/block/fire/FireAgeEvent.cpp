#include "ila/event/block/fire/FireAgeEvent.h"
#include "ila/base/Gloabl.i.h"
#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>
#include <mc/deps/nbt/CompoundTag.h>

namespace ila::block::inline fire {

void FireAgeEvent::serialize(CompoundTag& nbt) const {
    FireEvent::serialize(nbt);
    reflection::serialize_to(nbt["prev_age"], mPrevAge).value();
    reflection::serialize_to(nbt["new_age"], mNewAge).value();
}

void FireAgeEvent::deserialize(CompoundTag const& nbt) {
    FireEvent::deserialize(nbt);
    reflection::deserialize(mNewAge, nbt["new_age"]).value();
}

} // namespace ila::block::inline fire