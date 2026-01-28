#include "ila/event/minecraft/block/actor/BlockActorEvent.h"
#include "ila/base/Gloabl.h"

namespace ila::mc::inline world::inline level::inline block::inline actor
{

void BlockActorEvent::serialize(CompoundTag& nbt) const
{
    Event::serialize(nbt);
    nbt["self"] = serializeRefObj(self());
}

BlockActor& BlockActorEvent::self() const { return mSelf; }

} // namespace ila::mc::inline world::inline level::inline block::inline actor