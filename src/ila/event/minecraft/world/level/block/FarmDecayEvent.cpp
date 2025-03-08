#include "ila/event/minecraft/world/level/block/FarmDecayEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/block/FarmBlock.h>

namespace ila::mc::inline world::inline level::inline block
{

void FarmDecayBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]          = ListTag { pos().x, pos().y, pos().z };
    nbt["dimid"]        = getDimensionName(blockSource());
    nbt["actor"]        = serializeRefObj(actor());
    nbt["fallDistance"] = fallDistance();
}
void FarmDecayBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x        = nbt["pos"][0];
    pos().y        = nbt["pos"][1];
    pos().z        = nbt["pos"][2];
    fallDistance() = nbt["fallDistance"];
}
BlockPos& FarmDecayBeforeEvent::pos() const { return mPos; }
Actor*&   FarmDecayBeforeEvent::actor() const { return mActor; }
float&    FarmDecayBeforeEvent::fallDistance() const { return mFallDistance; }

void FarmDecayAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]          = ListTag { pos().x, pos().y, pos().z };
    nbt["dimid"]        = getDimensionName(blockSource());
    nbt["actor"]        = serializeRefObj(actor());
    nbt["fallDistance"] = fallDistance();
}
BlockPos const& FarmDecayAfterEvent::pos() const { return mPos; }
Actor* const&   FarmDecayAfterEvent::actor() const { return mActor; }
float const&    FarmDecayAfterEvent::fallDistance() const { return mFallDistance; }

LL_TYPE_INSTANCE_HOOK(
    FarmDecayEventHook,
    HookPriority::Normal,
    FarmBlock,
    &FarmBlock::$transformOnFall,
    void,
    BlockSource&    pRegion,
    BlockPos const& pPos,
    Actor*          pActor,
    float           pFallDistance
)
{
    auto beforeEvent = FarmDecayBeforeEvent(pRegion, const_cast<BlockPos&>(pPos), pActor, pFallDistance);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pRegion, pPos, pActor, pFallDistance);
    LLEventBus.publish(FarmDecayAfterEvent(pRegion, pPos, pActor, pFallDistance));
}

Event_Hook_Factory(FarmDecay, <FarmDecayEventHook>);

} // namespace ila::mc::inline world::inline level::inline block