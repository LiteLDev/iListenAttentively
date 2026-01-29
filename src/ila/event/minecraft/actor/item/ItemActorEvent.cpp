#include "ila/event/minecraft/actor/item/ItemActorEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/entity/ActorEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/item/ItemActor.h>

namespace ila::mc::inline actor::inline item
{

void ItemActorEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["self"] = serializeRefObj(self());
}

ItemActor& ItemActorEvent::self() const { return static_cast<ItemActor&>(ActorEvent::self()); }

} // namespace ila::mc::inline actor::inline item