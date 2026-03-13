#include "ila/event/block/LiquidFlowEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/service/Bedrock.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/server/ServerInstance.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/BedrockBlockNames.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/LiquidBlock.h>
#include <mc/world/level/material/Material.h>
#include <mc/world/level/material/MaterialType.h>
#include <thread>

namespace ila::mc::inline block
{

void LiquidFlowBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]         = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["dimId"]       = getDimensionName(blockSource());
    nbt["depth"]       = mDepth;
    nbt["flowFromPos"] = ListTag { mFlowFromPos.x, mFlowFromPos.y, mFlowFromPos.z };
}
void LiquidFlowBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mPos.x = nbt["pos"][0];
    mPos.y = nbt["pos"][1];
    mPos.z = nbt["pos"][2];
    mDepth = nbt["depth"];
}

void LiquidFlowAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]         = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["dimId"]       = getDimensionName(blockSource());
    nbt["depth"]       = mDepth;
    nbt["flowFromPos"] = ListTag { mFlowFromPos.x, mFlowFromPos.y, mFlowFromPos.z };
}

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
    if (std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
    {
        return origin(pRegion, pPos, pNeighbor, pFlowFromPos, pFlowFromDirection);
    }
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

} // namespace ila::mc::inline block