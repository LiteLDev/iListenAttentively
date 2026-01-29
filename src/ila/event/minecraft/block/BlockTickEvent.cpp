#include "ila/event/minecraft/block/BlockTickEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/world/WorldEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/block/Block.h>

namespace ila::mc::inline block
{

void BlockTickBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]    = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["dimId"]  = getDimensionName(blockSource());
    nbt["random"] = serializeRefObj(mRandom);
}
void BlockTickBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mPos.x = nbt["pos"][0];
    mPos.y = nbt["pos"][1];
    mPos.z = nbt["pos"][2];
}

void BlockTickAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]    = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["dimId"]  = getDimensionName(blockSource());
    nbt["random"] = serializeRefObj(mRandom);
}

LL_TYPE_INSTANCE_HOOK(
    BlockTickEventHook,
    HookPriority::Normal,
    Block,
    &Block::randomTick,
    void,
    BlockSource&    pRegion,
    BlockPos const& pPos,
    Random&         pRandom
)
{
    auto beforeEvent = BlockTickBeforeEvent(pRegion, const_cast<BlockPos&>(pPos), pRandom);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pRegion, pPos, pRandom);
    LLEventBus.publish(BlockTickAfterEvent(pRegion, pPos, pRandom));
}

Event_Hook_Factory(BlockTick, <BlockTickEventHook>);

} // namespace ila::mc::inline block