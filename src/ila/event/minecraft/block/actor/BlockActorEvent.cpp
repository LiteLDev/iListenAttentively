#include "ila/event/minecraft/block/actor/BlockActorEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Event.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/level/block/actor/BlockActor.h>

namespace ila::mc::inline block::inline actor
{

void BlockActorEvent::serialize(CompoundTag& nbt) const
{
    Event::serialize(nbt);
    nbt["self"] = serializeRefObj(mSelf);
}

} // namespace ila::mc::inline block::inline actor