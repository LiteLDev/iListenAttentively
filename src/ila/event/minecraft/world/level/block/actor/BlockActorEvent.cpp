#include "ila/event/minecraft/world/level/block/actor/BlockActorEvent.h"
#include "ila/base/Gloabl.h"

namespace ila::event::inline block::actor
{

void BlockActorEvent::serialize(CompoundTag& nbt) const
{
    Event::serialize(nbt);
    nbt["self"] = serializeRefObj(self());
}

BlockActor& BlockActorEvent::self() const { return mSelf; }

} // namespace ila::event::inline block::actor