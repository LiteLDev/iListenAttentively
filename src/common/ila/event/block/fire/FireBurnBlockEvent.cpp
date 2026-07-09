#include "ila/event/block/fire/FireBurnBlockEvent.h"
#include "ila/base/Gloabl.i.h"
#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>
#include <mc/deps/nbt/CompoundTag.h>

namespace ila::block::inline fire {

void FireBurnBlockEvent::serialize(CompoundTag& nbt) const {
    FireEvent::serialize(nbt);
    reflection::serialize_to(nbt["block_pos"], mBlockPos).value();
}

} // namespace ila::block::inline fire