#include "ila/event/minecraft/block/actor/BlockActorTick.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/world/WorldEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/block/actor/BlockActor.h>

namespace ila::mc::inline block::inline actor
{

void BlockActorTickBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["blockActor"] = serializeRefObj(blockActor());
    nbt["dimId"]      = getDimensionName(blockSource());
}
BlockActor& BlockActorTickBeforeEvent::blockActor() const { return mBlockActor; }

void BlockActorTickAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["blockActor"] = serializeRefObj(blockActor());
    nbt["dimId"]      = getDimensionName(blockSource());
}
BlockActor& BlockActorTickAfterEvent::blockActor() const { return mBlockActor; }

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

} // namespace ila::mc::inline block::inline actor