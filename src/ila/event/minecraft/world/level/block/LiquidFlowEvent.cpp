#include "ila/event/minecraft/world/level/block/LiquidFlowEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/block/LiquidBlockDynamic.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/material/Material.h>

namespace ila::mc::inline world::inline level::inline block
{

void LiquidFlowBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]               = ListTag { pos().x, pos().y, pos().z };
    nbt["dimId"]             = getDimensionName(blockSource());
    nbt["depth"]       = depth();
    nbt["preserveExisting"] = preserveExisting();
}
void LiquidFlowBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x             = nbt["pos"][0];
    pos().y             = nbt["pos"][1];
    pos().z             = nbt["pos"][2];
    depth()     = nbt["depth"];
    preserveExisting()     = nbt["preserveExisting"];
}
BlockPos& LiquidFlowBeforeEvent::pos() const { return mPos; }
int& LiquidFlowBeforeEvent::depth() const { return mDepth; }
bool&    LiquidFlowBeforeEvent::preserveExisting() const { return mPreserveExisting; }

void LiquidFlowAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]               = ListTag { pos().x, pos().y, pos().z };
    nbt["dimId"]             = getDimensionName(blockSource());
    nbt["depth"]       = depth();
    nbt["preserveExisting"] = preserveExisting();
}
BlockPos const& LiquidFlowAfterEvent::pos() const { return mPos; }
int const&      LiquidFlowAfterEvent::depth() const { return mDepth; }
bool const&     LiquidFlowAfterEvent::preserveExisting() const { return mPreserveExisting; }

LL_TYPE_INSTANCE_HOOK(
    LiquidFlowEventHook,
    HookPriority::Normal,
    LiquidBlockDynamic,
    &LiquidBlockDynamic::_spread,
    void,
    BlockSource&    pRegion,
    BlockPos const& pPos,
    int             pDepth,
    bool            pPreserveExisting
)
{
    if (pRegion.getLevel().isClientSide() && !pRegion.mAllowTickingChanges)
    {
        return origin(pRegion, pPos, pDepth, pPreserveExisting);
    }
    auto beforeEvent = LiquidFlowBeforeEvent(pRegion, const_cast<BlockPos&>(pPos), pDepth, pPreserveExisting);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pRegion, pPos, pDepth, pPreserveExisting);
    if (pRegion.getBlock(pPos).getLegacyBlock().mMaterial.mLiquid)
    {
        LLEventBus.publish(LiquidFlowAfterEvent(pRegion, pPos, pDepth, pPreserveExisting));
    }
}

Event_Hook_Factory(LiquidFlow, <LiquidFlowEventHook>);

} // namespace ila::mc::inline world::inline level::inline block