#include "ila/event/minecraft/world/SpawnWanderingTraderEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/actor/ai/village/WanderingTraderScheduler.h>

namespace ila::mc::inline world
{

void SpawnWanderingTraderBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]   = ListTag { pos().x, pos().y, pos().z };
    nbt["dimid"] = getDimensionName(blockSource());
}
void SpawnWanderingTraderBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x = nbt["pos"][0];
    pos().y = nbt["pos"][1];
    pos().z = nbt["pos"][2];
}
BlockPos& SpawnWanderingTraderBeforeEvent::pos() const { return mPos; }

void SpawnWanderingTraderAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]   = ListTag { pos().x, pos().y, pos().z };
    nbt["dimid"] = getDimensionName(blockSource());
}
BlockPos const& SpawnWanderingTraderAfterEvent::pos() const { return mPos; }

LL_TYPE_INSTANCE_HOOK(
    SpawnWanderingTraderEventHook,
    HookPriority::Normal,
    WanderingTraderScheduler,
    &WanderingTraderScheduler::_spawnWanderingTraderAtPos,
    void,
    BlockPos const& pPos,
    BlockSource&    pRegion
)
{
    auto beforeEvent = SpawnWanderingTraderBeforeEvent(pRegion, const_cast<BlockPos&>(pPos));
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pPos, pRegion);
    LLEventBus.publish(SpawnWanderingTraderAfterEvent(pRegion, pPos));
}

Event_Hook_Factory(SpawnWanderingTrader, <SpawnWanderingTraderEventHook>);

} // namespace ila::mc::inline world