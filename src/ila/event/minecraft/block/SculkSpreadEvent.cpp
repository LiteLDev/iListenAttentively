#include "ila/event/minecraft/block/SculkSpreadEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/WorldBlockTarget.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/MultifaceSpreader.h>

namespace ila::mc::inline world::inline level::inline block
{

void SculkSpreadBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["selfPos"]     = ListTag { selfPos().x, selfPos().y, selfPos().z };
    nbt["selfBlock"]   = serializeRefObj(selfBlock());
    nbt["selfFace"]    = selfFace();
    nbt["targetPos"]   = ListTag { targetPos().x, targetPos().y, targetPos().z };
    nbt["targetBlock"] = serializeRefObj(targetBlock());
    nbt["targetFace"]  = targetFace();
    nbt["facing"]      = facing();
    nbt["dimId"]       = getDimensionName(blockSource());
}
void SculkSpreadBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    selfPos().x   = nbt["selfPos"][0];
    selfPos().y   = nbt["selfPos"][1];
    selfPos().z   = nbt["selfPos"][2];
    selfFace()    = nbt["selfFace"];
    targetPos().x = nbt["targetPos"][0];
    targetPos().y = nbt["targetPos"][1];
    targetPos().z = nbt["targetPos"][2];
    targetFace()  = nbt["targetFace"];
    facing()      = nbt["facing"];
}
BlockPos& SculkSpreadBeforeEvent::selfPos() const { return mSelfPos; }
Block&    SculkSpreadBeforeEvent::selfBlock() const { return mSelfBlock; }
uchar&    SculkSpreadBeforeEvent::selfFace() const { return mSelfFace; }
BlockPos& SculkSpreadBeforeEvent::targetPos() const { return mTargetPos; }
Block&    SculkSpreadBeforeEvent::targetBlock() const { return mTargetBlock; }
uchar&    SculkSpreadBeforeEvent::targetFace() const { return mTargetFace; }
uchar&    SculkSpreadBeforeEvent::facing() const { return mFacing; }


void SculkSpreadAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["selfPos"]     = ListTag { selfPos().x, selfPos().y, selfPos().z };
    nbt["selfBlock"]   = serializeRefObj(selfBlock());
    nbt["selfFace"]    = selfFace();
    nbt["targetPos"]   = ListTag { targetPos().x, targetPos().y, targetPos().z };
    nbt["targetBlock"] = serializeRefObj(targetBlock());
    nbt["targetFace"]  = targetFace();
    nbt["facing"]      = facing();
    nbt["dimId"]       = getDimensionName(blockSource());
}
BlockPos const& SculkSpreadAfterEvent::selfPos() const { return mSelfPos; }
Block const&    SculkSpreadAfterEvent::selfBlock() const { return mSelfBlock; }
uchar const&    SculkSpreadAfterEvent::selfFace() const { return mSelfFace; }
BlockPos const& SculkSpreadAfterEvent::targetPos() const { return mTargetPos; }
Block const&    SculkSpreadAfterEvent::targetBlock() const { return mTargetBlock; }
uchar const&    SculkSpreadAfterEvent::targetFace() const { return mTargetFace; }
uchar const&    SculkSpreadAfterEvent::facing() const { return mFacing; }

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

} // namespace ila::mc::inline world::inline level::inline block