#include "ila/event/block/fire/FireLightCampfireEvent.h"
#include "ila/base/Gloabl.i.h"
#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>
#include <mc/nbt/CompoundTag.h>

namespace ila::block::inline fire {

void FireLightCampfireEvent::serialize(CompoundTag& nbt) const {
    FireEvent::serialize(nbt);
    reflection::serialize_to(nbt["campfire_pos"], mTNTPos).value();
}

} // namespace ila::block::inline fire