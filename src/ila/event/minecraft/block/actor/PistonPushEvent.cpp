#include "ila/event/minecraft/block/actor/PistonPushEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/block/actor/PistonBlockActor.h>

namespace ila::mc::inline block::inline actor
{

void PistonPushBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pistonPos"]        = ListTag { mPistonPos.x, mPistonPos.y, mPistonPos.z };
    nbt["pushPos"]          = ListTag { mPushPos.x, mPushPos.y, mPushPos.z };
    nbt["branchFacing"]     = mBranchFacing;
    nbt["pistonMoveFacing"] = mPistonMoveFacing;
    nbt["dimId"]            = getDimensionName(blockSource());
}
void PistonPushBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mPushPos.x        = nbt["pushPos"];
    mPushPos.y        = nbt["pushPos"];
    mPushPos.z        = nbt["pushPos"];
    mBranchFacing     = nbt["branchFacing"];
    mPistonMoveFacing = nbt["pistonMoveFacing"];
}

void PistonPushAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pistonPos"]        = ListTag { mPistonPos.x, mPistonPos.y, mPistonPos.z };
    nbt["pushPos"]          = ListTag { mPushPos.x, mPushPos.y, mPushPos.z };
    nbt["dimId"]            = getDimensionName(blockSource());
    nbt["branchFacing"]     = mBranchFacing;
    nbt["pistonMoveFacing"] = mPistonMoveFacing;
}

LL_TYPE_INSTANCE_HOOK(
    PistonPushEventHook,
    HookPriority::Normal,
    PistonBlockActor,
    &PistonBlockActor::_attachedBlockWalker,
    bool,
    BlockSource&    pRegion,
    BlockPos const& pPos,
    uchar           pBranchFacing,
    uchar           pPistonMoveFacing
)
{
    auto beforeEvent = PistonPushBeforeEvent(
        pRegion,
        mPosition,
        const_cast<BlockPos&>(pPos),
        pBranchFacing,
        pPistonMoveFacing
    );
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return false; }
    auto result = origin(pRegion, pPos, pBranchFacing, pPistonMoveFacing);
    if (result)
    {
        LLEventBus.publish(PistonPushAfterEvent(pRegion, mPosition, pPos, pBranchFacing, pPistonMoveFacing));
    }
    return result;
}

Event_Hook_Factory(PistonPush, <PistonPushEventHook>);

} // namespace ila::mc::inline block::inline actor
