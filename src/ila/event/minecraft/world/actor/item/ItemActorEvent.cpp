#include "ila/event/minecraft/world/actor/item/ItemActorEvent.h"
#include "ila/base/Gloabl.h"

namespace ila::mc::inline world::inline actor::inline item {

void ItemActorEvent::serialize(CompoundTag& nbt) const {
    ActorEvent::serialize(nbt);
    nbt["self"] = serializeRefObj(self());
}

ItemActor& ItemActorEvent::self() const { return static_cast<ItemActor&>(ActorEvent::self()); }

} // namespace ila::mc::inline world::inline actor::inline item