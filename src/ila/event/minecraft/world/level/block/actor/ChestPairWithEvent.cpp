#include "ila/event/minecraft/world/level/block/actor/ChestPairWithEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/BlockPos.h>

namespace ila::mc::inline world::inline level::inline block::inline actor
{

void ChestPairWithBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["chest"]    = serializeRefObj(getChest());
    nbt["position"] = ListTag { getPosition().x, getPosition().y, getPosition().z };
    nbt["dimid"]      = getDimensionName(blockSource());
}
void ChestPairWithBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    getPosition().x = nbt["position"][0];
    getPosition().y = nbt["position"][1];
    getPosition().z = nbt["position"][2];
}
ChestBlockActor& ChestPairWithBeforeEvent::getChest() const { return mChest; }
BlockPos&        ChestPairWithBeforeEvent::getPosition() const { return mPosition; }

void ChestPairWithAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["chest"]    = serializeRefObj(getChest());
    nbt["position"] = ListTag { getPosition().x, getPosition().y, getPosition().z };
    nbt["dimid"]      = getDimensionName(blockSource());
}
ChestBlockActor& ChestPairWithAfterEvent::getChest() const { return mChest; }
BlockPos const&  ChestPairWithAfterEvent::getPosition() const { return mPosition; }

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