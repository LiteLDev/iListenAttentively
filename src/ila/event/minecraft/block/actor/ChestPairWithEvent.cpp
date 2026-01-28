#include "ila/event/minecraft/block/actor/ChestPairWithEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/BlockPos.h>

namespace ila::mc::inline world::inline level::inline block::inline actor
{

void ChestPairWithBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["chest"]    = serializeRefObj(chest());
    nbt["position"] = ListTag { pos().x, pos().y, pos().z };
    nbt["dimId"]      = getDimensionName(blockSource());
}
void ChestPairWithBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x = nbt["position"][0];
    pos().y = nbt["position"][1];
    pos().z = nbt["position"][2];
}
ChestBlockActor& ChestPairWithBeforeEvent::chest() const { return mChest; }
BlockPos&        ChestPairWithBeforeEvent::pos() const { return mPosition; }

void ChestPairWithAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["chest"]    = serializeRefObj(chest());
    nbt["position"] = ListTag { pos().x, pos().y, pos().z };
    nbt["dimId"]      = getDimensionName(blockSource());
}
ChestBlockActor& ChestPairWithAfterEvent::chest() const { return mChest; }
BlockPos const&  ChestPairWithAfterEvent::pos() const { return mPosition; }

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

} // namespace ila::mc::inline world::inline level::inline block::inline actor