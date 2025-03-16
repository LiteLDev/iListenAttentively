#include "ila/event/minecraft/world/level/block/BlockTickEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/block/Block.h>

namespace ila::mc::inline world::inline level::inline block
{

void BlockTickBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]    = ListTag { pos().x, pos().y, pos().z };
    nbt["dimId"]  = getDimensionName(blockSource());
    nbt["random"] = serializeRefObj(random());
}
void BlockTickBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x = nbt["pos"][0];
    pos().y = nbt["pos"][1];
    pos().z = nbt["pos"][2];
}
BlockPos& BlockTickBeforeEvent::pos() const { return mPos; }
Random&   BlockTickBeforeEvent::random() const { return mRandom; }

void BlockTickAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]    = ListTag { pos().x, pos().y, pos().z };
    nbt["dimId"]  = getDimensionName(blockSource());
    nbt["random"] = serializeRefObj(random());
}
BlockPos const& BlockTickAfterEvent::pos() const { return mPos; }
Random const&   BlockTickAfterEvent::random() const { return mRandom; }

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

} // namespace ila::mc::inline world::inline level::inline block