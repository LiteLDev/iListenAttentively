#include "ila/event/minecraft/block/SculkBlockGrowthEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/WorldBlockTarget.h>
#include <mc/world/level/block/SculkBlockBehavior.h>

namespace ila::mc::inline world
{

void SculkBlockGrowthBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]   = ListTag { pos().x, pos().y, pos().z };
    nbt["dimId"] = getDimensionName(blockSource());
}
void SculkBlockGrowthBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x = nbt["pos"][0];
    pos().y = nbt["pos"][1];
    pos().z = nbt["pos"][2];
}
BlockPos& SculkBlockGrowthBeforeEvent::pos() const { return mPos; }

void SculkBlockGrowthAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]   = ListTag { pos().x, pos().y, pos().z };
    nbt["dimId"] = getDimensionName(blockSource());
}
BlockPos const& SculkBlockGrowthAfterEvent::pos() const { return mPos; }

LL_STATIC_HOOK(
    SculkBlockGrowthEventHook,
    HookPriority::Normal,
    &SculkBlockBehavior::_placeGrowthAt,
    void,
    IBlockWorldGenAPI& pTarget,
    BlockSource*       pRegion,
    BlockPos const&    pPos,
    Random&            pRandom,
    SculkSpreader&     pSculkSpreader
)
{
    if (pRegion == nullptr) { return origin(pTarget, pRegion, pPos, pRandom, pSculkSpreader); }
    auto beforeEvent = SculkBlockGrowthBeforeEvent(*pRegion, const_cast<BlockPos&>(pPos));
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pTarget, pRegion, pPos, pRandom, pSculkSpreader);
    LLEventBus.publish(SculkBlockGrowthAfterEvent(*pRegion, pPos));
}

Event_Hook_Factory(SculkBlockGrowth, <SculkBlockGrowthEventHook>);

} // namespace ila::mc::inline world