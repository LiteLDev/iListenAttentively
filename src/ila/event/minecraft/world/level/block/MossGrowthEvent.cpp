#include "ila/event/minecraft/world/level/block/MossGrowthEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/WorldBlockTarget.h>
#include <mc/world/level/levelgen/feature/VegetationPatchFeature.h>

namespace ila::mc::inline world::inline level::inline block
{

void MossGrowthBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]    = ListTag { pos().x, pos().y, pos().z };
    nbt["dimId"]  = getDimensionName(blockSource());
    nbt["random"] = serializeRefObj(random());
}
void MossGrowthBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x = nbt["pos"][0];
    pos().y = nbt["pos"][1];
    pos().z = nbt["pos"][2];
}
BlockPos& MossGrowthBeforeEvent::pos() const { return mPos; };
Random&   MossGrowthBeforeEvent::random() const { return mRandom; };

void MossGrowthAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]       = ListTag { pos().x, pos().y, pos().z };
    nbt["dimId"]     = getDimensionName(blockSource());
    nbt["random"]    = serializeRefObj(random());
    nbt["targetPos"] = ListTag { getTargetPos()->x, getTargetPos()->y, getTargetPos()->z };
}
void MossGrowthAfterEvent::deserialize(CompoundTag const& nbt)
{
    WorldEvent::deserialize(nbt);
    auto& pos      = nbt["targetPoss"].get<ListTag>();
    getTargetPos() = { static_cast<int>(pos[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2]) };
}
BlockPos const&          MossGrowthAfterEvent::pos() const { return mPos; };
Random const&            MossGrowthAfterEvent::random() const { return mRandom; };
std::optional<BlockPos>& MossGrowthAfterEvent::getTargetPos() const { return mTargetPos; };

LL_TYPE_INSTANCE_HOOK(
    MossGrowthEventHook,
    HookPriority::Normal,
    VegetationPatchFeature,
    &VegetationPatchFeature::$place,
    std::optional<::BlockPos>,
    IFeature::PlacementContext const& context
)
{
    auto& region      = static_cast<WorldBlockTarget&>(context.mTarget).mBlockSource;
    auto  beforeEvent = MossGrowthBeforeEvent(region, const_cast<BlockPos&>(*context.mPos), context.mRandom);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return {}; }
    auto result = origin(context);
    if (result)
    {
        LLEventBus.publish(
            MossGrowthAfterEvent(region, const_cast<BlockPos&>(*context.mPos), context.mRandom, result)
        );
    }
    return result;
}

Event_Hook_Factory(MossGrowth, <MossGrowthEventHook>);

} // namespace ila::mc::inline world::inline level::inline block
