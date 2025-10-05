#include "ila/event/minecraft/world/level/block/LiquidFlowEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/BedrockBlockNames.h>
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
    nbt["flowFromPos"]       = ListTag { flowFromPos().x, flowFromPos().y, flowFromPos().z };
}
void LiquidFlowBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x            = nbt["pos"][0];
    pos().y            = nbt["pos"][1];
    pos().z            = nbt["pos"][2];
    depth()            = nbt["depth"];
}
BlockPos&       LiquidFlowBeforeEvent::pos() const { return mPos; }
int&            LiquidFlowBeforeEvent::depth() const { return mDepth; }
BlockPos const& LiquidFlowBeforeEvent::flowFromPos() const { return mFlowFromPos; }

void LiquidFlowAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]              = ListTag { pos().x, pos().y, pos().z };
    nbt["dimId"]            = getDimensionName(blockSource());
    nbt["depth"]            = depth();
    nbt["flowFromPos"]      = ListTag { flowFromPos().x, flowFromPos().y, flowFromPos().z };
}
BlockPos const& LiquidFlowAfterEvent::pos() const { return mPos; }
int const&      LiquidFlowAfterEvent::depth() const { return mDepth; }
BlockPos const& LiquidFlowAfterEvent::flowFromPos() const { return mFlowFromPos; }

bool operator==(Material const& lhs, Material const& rhs)
{
    return lhs.mType == rhs.mType && lhs.mNeverBuildable == rhs.mNeverBuildable && lhs.mLiquid == rhs.mLiquid
           && lhs.mBlocksMotion == rhs.mBlocksMotion && lhs.mBlocksPrecipitation == rhs.mBlocksPrecipitation
           && lhs.mSolid == rhs.mSolid && lhs.mSuperHot == rhs.mSuperHot;
}

LL_TYPE_INSTANCE_HOOK(
    LiquidFlowEventHook,
    HookPriority::Normal,
    LiquidBlock,
    &LiquidBlock::_trySpreadTo,
    void,
    BlockSource&    pRegion,
    BlockPos const& pPos,
    int             pNeighbor,
    BlockPos const& pFlowFromPos,
    uchar           pFlowFromDirection
)
{
    if (pPos.y < pRegion.getMinHeight() || !pRegion.hasBlock(pPos)) { return; }
    if (auto& block = pRegion.getLiquidBlock(pPos).mBlockType;
        block->mMaterial == mMaterial || block->mMaterial.mType == MaterialType::Lava
        || _isLiquidBlocking(pRegion, pPos, pFlowFromPos, pFlowFromDirection))
    {
        return;
    }
    auto beforeEvent = LiquidFlowBeforeEvent(pRegion, const_cast<BlockPos&>(pPos), pNeighbor, pFlowFromPos);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pRegion, pPos, pNeighbor, pFlowFromPos, pFlowFromDirection);
    if (pRegion.getBlock(pPos).mBlockType->mMaterial.mLiquid)
    {
        LLEventBus.publish(LiquidFlowAfterEvent(pRegion, pPos, pNeighbor, pFlowFromPos));
    }
}

Event_Hook_Factory(LiquidFlow, <LiquidFlowEventHook>);

} // namespace ila::mc::inline world::inline level::inline block