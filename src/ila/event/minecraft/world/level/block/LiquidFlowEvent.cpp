#include "ila/event/minecraft/world/level/block/LiquidFlowEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/LiquidBlock.h>
#include <mc/world/level/material/Material.h>

namespace ila::mc::inline world::inline level::inline block
{

void LiquidFlowBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]               = ListTag { pos().x, pos().y, pos().z };
    nbt["dimId"]             = getDimensionName(blockSource());
    nbt["depth"]             = depth();
    nbt["preserveExisting"]  = preserveExisting();
    nbt["flowFromPos"]       = ListTag { flowFromPos().x, flowFromPos().y, flowFromPos().z };
}
void LiquidFlowBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x            = nbt["pos"][0];
    pos().y            = nbt["pos"][1];
    pos().z            = nbt["pos"][2];
    depth()            = nbt["depth"];
    preserveExisting() = nbt["preserveExisting"];
}
BlockPos&       LiquidFlowBeforeEvent::pos() const { return mPos; }
int&            LiquidFlowBeforeEvent::depth() const { return mDepth; }
bool&           LiquidFlowBeforeEvent::preserveExisting() const { return mPreserveExisting; }
BlockPos const& LiquidFlowBeforeEvent::flowFromPos() const { return mFlowFromPos; }

void LiquidFlowAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]              = ListTag { pos().x, pos().y, pos().z };
    nbt["dimId"]            = getDimensionName(blockSource());
    nbt["depth"]            = depth();
    nbt["preserveExisting"] = preserveExisting();
    nbt["flowFromPos"]      = ListTag { flowFromPos().x, flowFromPos().y, flowFromPos().z };
}
BlockPos const& LiquidFlowAfterEvent::pos() const { return mPos; }
int const&      LiquidFlowAfterEvent::depth() const { return mDepth; }
bool const&     LiquidFlowAfterEvent::preserveExisting() const { return mPreserveExisting; }
BlockPos const& LiquidFlowAfterEvent::flowFromPos() const { return mFlowFromPos; }

static BlockPos* mFlowFromPos = nullptr;

LL_TYPE_INSTANCE_HOOK(
    LiquidFlowEventHook1,
    HookPriority::Normal,
    LiquidBlock,
    &LiquidBlock::_trySpreadTo,
    void,
    BlockSource&    region,
    BlockPos const& pos,
    int             neighbor,
    BlockPos const& flowFromPos,
    uchar           flowFromDirection
)
{
    mFlowFromPos = const_cast<BlockPos*>(&flowFromPos);
    origin(region, pos, neighbor, flowFromPos, flowFromDirection);
    mFlowFromPos = nullptr;
}

LL_TYPE_INSTANCE_HOOK(
    LiquidFlowEventHook2,
    HookPriority::Normal,
    LiquidBlock,
    &LiquidBlock::_spread,
    void,
    BlockSource&    pRegion,
    BlockPos const& pPos,
    int             pDepth,
    bool            pPreserveExisting
)
{
    if (!mFlowFromPos) { return origin(pRegion, pPos, pDepth, pPreserveExisting); }
    auto beforeEvent =
        LiquidFlowBeforeEvent(pRegion, const_cast<BlockPos&>(pPos), pDepth, pPreserveExisting, *mFlowFromPos);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pRegion, pPos, pDepth, pPreserveExisting);
    if (pRegion.getBlock(pPos).mBlockType->mMaterial.mLiquid)
    {
        LLEventBus.publish(LiquidFlowAfterEvent(pRegion, pPos, pDepth, pPreserveExisting, *mFlowFromPos));
    }
}

Event_Hook_Factory(LiquidFlow, <LiquidFlowEventHook1, LiquidFlowEventHook2>);

} // namespace ila::mc::inline world::inline level::inline block