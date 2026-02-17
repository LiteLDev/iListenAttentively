#include "ila/event/minecraft/block/MossGrowthEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/world/WorldEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/IBlockWorldGenAPI.h>
#include <mc/world/level/WorldBlockTarget.h>
#include <mc/world/level/levelgen/feature/VegetationPatchFeature.h>
#include <vector>

namespace ila::mc::inline block
{

void MossGrowthBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]     = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["dimId"]   = getDimensionName(blockSource());
    nbt["random"]  = serializeRefObj(mRandom);
    nbt["xRadius"] = mXRadius;
    nbt["zRadius"] = mZRadius;
}
void MossGrowthBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mPos.x   = nbt["pos"][0];
    mPos.y   = nbt["pos"][1];
    mPos.z   = nbt["pos"][2];
    mXRadius = nbt["xRadius"];
    mZRadius = nbt["zRadius"];
};;;;

void MossGrowthAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]        = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["dimId"]      = getDimensionName(blockSource());
    nbt["random"]     = serializeRefObj(mRandom);
    nbt["xRadius"]    = mXRadius;
    nbt["zRadius"]    = mZRadius;
    nbt["targetPoss"] = ListTag {};
    for (auto const& pos : mTargetPoss) { nbt["targetPoss"].push_back(ListTag { pos.x, pos.y, pos.z }); }
}
void MossGrowthAfterEvent::deserialize(CompoundTag const& nbt)
{
    WorldEvent::deserialize(nbt);
    mTargetPoss.clear();
    for (auto& pos : nbt["targetPoss"].get<ListTag>())
    {
        mTargetPoss.push_back(
            { static_cast<int>(pos[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2]) }
        );
    }
};;;;;

LL_TYPE_INSTANCE_HOOK(
    MossGrowthEventHook,
    HookPriority::Normal,
    VegetationPatchFeature,
    &VegetationPatchFeature::_placeGroundPatch,
    std::vector<BlockPos>,
    IBlockWorldGenAPI& pTarget,
    Random&            pRandom,
    BlockPos const&    pPos,
    int                pXRadius,
    int                pZRadius
)
{
    auto& region = static_cast<WorldBlockTarget&>(pTarget).mBlockSource;
    auto  beforeEvent =
        MossGrowthBeforeEvent(region, const_cast<BlockPos&>(pPos), pRandom, pXRadius, pZRadius);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return {}; }
    auto result = origin(pTarget, pRandom, pPos, pXRadius, pZRadius);
    if (!result.empty())
    {
        LLEventBus.publish(MossGrowthAfterEvent(region, pPos, pRandom, pXRadius, pZRadius, result));
    }
    return result;
}

Event_Hook_Factory(MossGrowth, <MossGrowthEventHook>);

} // namespace ila::mc::inline block