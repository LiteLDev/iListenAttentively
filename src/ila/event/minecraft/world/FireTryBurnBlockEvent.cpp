#include "ila/event/minecraft/world/FireTryBurnBlockEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/block/FireBlock.h>

namespace ila::mc::inline world
{

void FireTryBurnBlockBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]   = ListTag { pos().x, pos().y, pos().z };
    nbt["dimId"] = getDimensionName(blockSource());
}
BlockPos const& FireTryBurnBlockBeforeEvent::pos() const { return mPos; }

void FireTryBurnBlockAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]   = ListTag { pos().x, pos().y, pos().z };
    nbt["dimId"] = getDimensionName(blockSource());
}
BlockPos const& FireTryBurnBlockAfterEvent::pos() const { return mPos; }

LL_TYPE_INSTANCE_HOOK(
    FireTryBurnBlockEventHook,
    HookPriority::Normal,
    FireBlock,
    &FireBlock::isValidFireLocation,
    bool,
    BlockSource&    region,
    BlockPos const& pos
)
{
    auto beforeEvent = FireTryBurnBlockBeforeEvent(region, pos);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return false; }
    auto result = origin(region, pos);
    if (result) { LLEventBus.publish(FireTryBurnBlockAfterEvent(region, pos)); }
    return result;
}

Event_Hook_Factory(FireTryBurnBlock, <FireTryBurnBlockEventHook>);

} // namespace ila::mc::inline world