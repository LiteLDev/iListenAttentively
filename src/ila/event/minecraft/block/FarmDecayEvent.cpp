#include "ila/event/minecraft/block/FarmDecayEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/world/WorldEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/IBlockSource.h>
#include <mc/world/level/block/FarmBlock.h>

namespace ila::mc::inline block
{

void FarmDecayBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]          = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["dimId"]        = getDimensionName(blockSource());
    nbt["actor"]        = serializeRefObj(mActor);
    nbt["fallDistance"] = mFallDistance;
}
void FarmDecayBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mPos.x        = nbt["pos"][0];
    mPos.y        = nbt["pos"][1];
    mPos.z        = nbt["pos"][2];
    mFallDistance = nbt["fallDistance"];
}

void FarmDecayAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]          = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["dimId"]        = getDimensionName(blockSource());
    nbt["actor"]        = serializeRefObj(mActor);
    nbt["fallDistance"] = mFallDistance;
}

LL_TYPE_INSTANCE_HOOK(
    FarmDecayEventHook,
    HookPriority::Normal,
    FarmBlock,
    &FarmBlock::$transformOnFall,
    void,
    BlockSource&    pRegion,
    BlockPos const& pPos,
    Actor*          pActor,
    float           pFallDistance
)
{
    auto beforeEvent = FarmDecayBeforeEvent(pRegion, const_cast<BlockPos&>(pPos), pActor, pFallDistance);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pRegion, pPos, pActor, pFallDistance);
    LLEventBus.publish(FarmDecayAfterEvent(pRegion, pPos, pActor, pFallDistance));
}

Event_Hook_Factory(FarmDecay, <FarmDecayEventHook>);

} // namespace ila::mc::inline block