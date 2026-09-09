#include "ila/event/minecraft/world/SculkBlockGrowthEvent.h"
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

// TODO: Check the behavior
LL_TYPE_INSTANCE_HOOK(
    SculkBlockGrowthEventHook,
    HookPriority::Normal,
    SculkBlockBehavior,
    &SculkBlockBehavior::$attemptUseCharge,
    int,
    IBlockWorldGenAPI& target,
    ::BlockSource*     region,
    ::BlockPos const&  originPos,
    ::BlockPos const&  pos,
    int                charge,
    int                idk1,
    ::Random&          random,
    ::SculkSpreader&   spreader,
    bool const         idk2
)
{
    if (region == nullptr)
    {
        return origin(target, region, originPos, pos, charge, idk1, random, spreader, idk2);
    }
    auto beforeEvent = SculkBlockGrowthBeforeEvent(*region, const_cast<BlockPos&>(pos));
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return 0; }
    int res = origin(target, region, originPos, pos, charge, idk1, random, spreader, idk2);
    LLEventBus.publish(SculkBlockGrowthAfterEvent(*region, pos));
    return res;
}

Event_Hook_Factory(SculkBlockGrowth, <SculkBlockGrowthEventHook>);

} // namespace ila::mc::inline world
