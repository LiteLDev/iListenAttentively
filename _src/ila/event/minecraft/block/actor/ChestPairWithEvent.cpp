#include "ila/event/block/actor/ChestPairWithEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/world/WorldEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/block/actor/ChestBlockActor.h>

namespace ila::mc::inline block::inline actor
{

void ChestPairWithBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["chest"]    = serializeRefObj(mChest);
    nbt["position"] = ListTag { mPosition.x, mPosition.y, mPosition.z };
    nbt["dimId"]    = getDimensionName(blockSource());
}
void ChestPairWithBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mPosition.x = nbt["position"][0];
    mPosition.y = nbt["position"][1];
    mPosition.z = nbt["position"][2];
}

void ChestPairWithAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["chest"]    = serializeRefObj(mChest);
    nbt["position"] = ListTag { mPosition.x, mPosition.y, mPosition.z };
    nbt["dimId"]    = getDimensionName(blockSource());
}

LL_TYPE_INSTANCE_HOOK(
    ChestPairWithEventHook,
    HookPriority::Normal,
    ChestBlockActor,
    &ChestBlockActor::_tryToPairWith,
    void,
    BlockSource&    region,
    BlockPos const& position
)
{
    auto beforeEvent = ChestPairWithBeforeEvent(region, *this, const_cast<BlockPos&>(position));
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(region, position);
    if (mLargeChestPaired) { LLEventBus.publish(ChestPairWithAfterEvent(region, *this, position)); }
}

Event_Hook_Factory(ChestPairWith, <ChestPairWithEventHook>);

} // namespace ila::mc::inline block::inline actor