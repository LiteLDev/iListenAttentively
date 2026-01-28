#include "ila/event/minecraft/block/actor/PistonPushEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/block/actor/PistonBlockActor.h>

namespace ila::mc::inline world
{

void PistonPushBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pistonPos"]        = ListTag { pistonPos().x, pistonPos().y, pistonPos().z };
    nbt["pushPos"]          = ListTag { pushPos().x, pushPos().y, pushPos().z };
    nbt["branchFacing"]     = branchFacing();
    nbt["pistonMoveFacing"] = pistonMoveFacing();
    nbt["dimId"]            = getDimensionName(blockSource());
}
void PistonPushBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pushPos().x        = nbt["pushPos"];
    pushPos().y        = nbt["pushPos"];
    pushPos().z        = nbt["pushPos"];
    branchFacing()     = nbt["branchFacing"];
    pistonMoveFacing() = nbt["pistonMoveFacing"];
}
BlockPos& PistonPushBeforeEvent::pistonPos() const { return mPistonPos; }
BlockPos& PistonPushBeforeEvent::pushPos() const { return mPushPos; }
uchar&    PistonPushBeforeEvent::branchFacing() const { return mBranchFacing; }
uchar&    PistonPushBeforeEvent::pistonMoveFacing() const { return mPistonMoveFacing; }

void PistonPushAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pistonPos"]        = ListTag { pistonPos().x, pistonPos().y, pistonPos().z };
    nbt["pushPos"]          = ListTag { pushPos().x, pushPos().y, pushPos().z };
    nbt["dimId"]            = getDimensionName(blockSource());
    nbt["branchFacing"]     = branchFacing();
    nbt["pistonMoveFacing"] = pistonMoveFacing();
}
BlockPos const& PistonPushAfterEvent::pistonPos() const { return mPistonPos; }
BlockPos const& PistonPushAfterEvent::pushPos() const { return mPushPos; }
uchar const&    PistonPushAfterEvent::branchFacing() const { return mBranchFacing; }
uchar const&    PistonPushAfterEvent::pistonMoveFacing() const { return mPistonMoveFacing; }

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

} // namespace ila::mc::inline world