#include "ila/event/minecraft/block/SculkSpreadEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/base/StdInt.h>
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/world/WorldEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/IBlockWorldGenAPI.h>
#include <mc/world/level/WorldBlockTarget.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/MultifaceSpreader.h>
#include <optional>
#include <utility>

namespace ila::mc::inline block
{

void SculkSpreadBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["selfPos"]     = ListTag { mSelfPos.x, mSelfPos.y, mSelfPos.z };
    nbt["selfBlock"]   = serializeRefObj(mSelfBlock);
    nbt["selfFace"]    = mSelfFace;
    nbt["targetPos"]   = ListTag { mTargetPos.x, mTargetPos.y, mTargetPos.z };
    nbt["targetBlock"] = serializeRefObj(mTargetBlock);
    nbt["targetFace"]  = mTargetFace;
    nbt["facing"]      = mFacing;
    nbt["dimId"]       = getDimensionName(blockSource());
}
void SculkSpreadBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mSelfPos.x   = nbt["selfPos"][0];
    mSelfPos.y   = nbt["selfPos"][1];
    mSelfPos.z   = nbt["selfPos"][2];
    mSelfFace    = nbt["selfFace"];
    mTargetPos.x = nbt["targetPos"][0];
    mTargetPos.y = nbt["targetPos"][1];
    mTargetPos.z = nbt["targetPos"][2];
    mTargetFace  = nbt["targetFace"];
    mFacing      = nbt["facing"];
}


void SculkSpreadAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["selfPos"]     = ListTag { mSelfPos.x, mSelfPos.y, mSelfPos.z };
    nbt["selfBlock"]   = serializeRefObj(mSelfBlock);
    nbt["selfFace"]    = mSelfFace;
    nbt["targetPos"]   = ListTag { mTargetPos.x, mTargetPos.y, mTargetPos.z };
    nbt["targetBlock"] = serializeRefObj(mTargetBlock);
    nbt["targetFace"]  = mTargetFace;
    nbt["facing"]      = mFacing;
    nbt["dimId"]       = getDimensionName(blockSource());
}

using ReturnType = std::optional<std::pair<BlockPos const, uchar const>>;

LL_TYPE_INSTANCE_HOOK(
    SculkSpreadEventHook,
    HookPriority::Normal,
    MultifaceSpreader,
    &MultifaceSpreader::getSpreadFromFaceTowardDirection,
    ReturnType,
    IBlockWorldGenAPI& pTarget,
    Block const&       pSelf,
    Block const&       pBlock,
    BlockPos const&    pPos,
    uchar              pFacing,
    uchar              pFace
)
{
    auto result = origin(pTarget, pSelf, pBlock, pPos, pFacing, pFace);
    if (result.has_value())
    {
        auto& region      = (static_cast<WorldBlockTarget&>(pTarget)).mBlockSource;
        auto  beforeEvent = SculkSpreadBeforeEvent(
            region,
            const_cast<BlockPos&>(pPos),
            const_cast<Block&>(pSelf),
            pFace,
            const_cast<BlockPos&>(result->first),
            const_cast<Block&>(pBlock),
            const_cast<uchar&>(result->second),
            pFacing
        );
        LLEventBus.publish(beforeEvent);
        if (beforeEvent.isCancelled()) { return std::nullopt; }
        LLEventBus.publish(
            SculkSpreadAfterEvent(region, pPos, pSelf, pFace, result->first, pBlock, result->second, pFacing)
        );
    }
    return result;
}

Event_Hook_Factory(SculkSpread, <SculkSpreadEventHook>);

} // namespace ila::mc::inline block