#include "ila/event/minecraft/world/SpawnWanderingTraderEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/actor/ai/village/WanderingTraderScheduler.h>
#include <mc/world/level/BlockPos.h>

namespace ila::mc::inline world
{

void SpawnWanderingTraderBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]   = ListTag { pos().x, pos().y, pos().z };
    nbt["dimId"] = getDimensionName(blockSource());
}
void SpawnWanderingTraderBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x = nbt["pos"][0];
    pos().y = nbt["pos"][1];
    pos().z = nbt["pos"][2];
}
BlockPos& SpawnWanderingTraderBeforeEvent::pos() const { return mPos; }

LL_TYPE_INSTANCE_HOOK(
    SpawnWanderingTraderEventHook,
    HookPriority::Normal,
    WanderingTraderScheduler,
    &WanderingTraderScheduler::_canSpawnAtPosition,
    bool,
    BlockPos const& pPos,
    BlockSource&    pRegion
)
{
    if (!origin(pPos, pRegion)) { return false; }
    auto beforeEvent = SpawnWanderingTraderBeforeEvent(pRegion, const_cast<BlockPos&>(pPos));
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return false; }
    return true;
}

Event_Hook_Factory_Base(SpawnWanderingTraderBefore, <SpawnWanderingTraderEventHook>);

} // namespace ila::mc::inline world