#include "ila/event/minecraft/world/level/block/LiquidTryFlowEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/block/LiquidBlockDynamic.h>

namespace ila::mc::inline world::inline level::inline block
{

void LiquidTryFlowBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]               = ListTag { pos().x, pos().y, pos().z };
    nbt["dimid"]             = getDimensionName(blockSource());
    nbt["flowFromPos"]       = ListTag { flowFromPos().x, flowFromPos().y, flowFromPos().z };
    nbt["flowFromDirection"] = flowFromDirection();
}
void LiquidTryFlowBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x             = nbt["pos"]["x"];
    pos().y             = nbt["pos"]["y"];
    pos().z             = nbt["pos"]["z"];
    flowFromPos().x     = nbt["flowFromPos"]["x"];
    flowFromPos().y     = nbt["flowFromPos"]["y"];
    flowFromPos().z     = nbt["flowFromPos"]["z"];
    flowFromDirection() = nbt["flowFromDirection"];
}
BlockPos& LiquidTryFlowBeforeEvent::pos() const { return mPos; }
BlockPos& LiquidTryFlowBeforeEvent::flowFromPos() const { return mFlowFromPos; }
uchar&    LiquidTryFlowBeforeEvent::flowFromDirection() const { return mFlowFromDirection; }

void LiquidTryFlowAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]               = ListTag { pos().x, pos().y, pos().z };
    nbt["dimid"]             = getDimensionName(blockSource());
    nbt["flowFromPos"]       = ListTag { flowFromPos().x, flowFromPos().y, flowFromPos().z };
    nbt["flowFromDirection"] = flowFromDirection();
}
BlockPos const& LiquidTryFlowAfterEvent::pos() const { return mPos; }
BlockPos const& LiquidTryFlowAfterEvent::flowFromPos() const { return mFlowFromPos; }
uchar const&    LiquidTryFlowAfterEvent::flowFromDirection() const { return mFlowFromDirection; }

LL_TYPE_INSTANCE_HOOK(
    LiquidTryFlowEventHook,
    HookPriority::Normal,
    LiquidBlockDynamic,
    &LiquidBlockDynamic::_isLiquidBlocking,
    bool,
    BlockSource&    pRegion,
    BlockPos const& pPos,
    BlockPos const& pFlowFromPos,
    uchar           pFlowFromDirection
)
{
    auto beforeEvent = LiquidTryFlowBeforeEvent(
        pRegion,
        const_cast<BlockPos&>(pPos),
        const_cast<BlockPos&>(pFlowFromPos),
        pFlowFromDirection
    );
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return true; }
    auto result = origin(pRegion, pPos, pFlowFromPos, pFlowFromDirection);
    if (!result)
    {
        LLEventBus.publish(LiquidTryFlowAfterEvent(pRegion, pPos, pFlowFromPos, pFlowFromDirection));
    }
    return result;
}

Event_Hook_Factory(LiquidTryFlow, <LiquidTryFlowEventHook>);

} // namespace ila::mc::inline world::inline level::inline block