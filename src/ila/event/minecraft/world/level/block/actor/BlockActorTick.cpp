#include "ila/event/minecraft/world/level/block/actor/BlockActorTick.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/block/actor/BlockActor.h>

namespace ila::mc::inline blockActor
{

void BlockActorTickBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["blockActor"] = serializeRefObj(getBlockActor());
}
BlockActor& BlockActorTickBeforeEvent::getBlockActor() const { return mBlockActor; }

void BlockActorTickAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["blockActor"] = serializeRefObj(getBlockActor());
}
BlockActor& BlockActorTickAfterEvent::getBlockActor() const { return mBlockActor; }

LL_TYPE_INSTANCE_HOOK(
    BlockActorTickEventHook,
    HookPriority::Normal,
    BlockActor,
    &BlockActor::$tick,
    void,
    BlockSource& pRegion
)
{
    auto beforeEvent = BlockActorTickBeforeEvent(pRegion, *this);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pRegion);
    LLEventBus.publish(BlockActorTickAfterEvent(pRegion, *this));
}

Event_Hook_Factory(BlockActorTick, <BlockActorTickEventHook>);

} // namespace ila::mc::inline world