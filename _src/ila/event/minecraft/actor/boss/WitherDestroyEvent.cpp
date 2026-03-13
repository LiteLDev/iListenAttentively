#include "ila/event/actor/boss/WitherDestroyEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/actor/boss/WitherBoss.h>
#include <mc/world/phys/AABB.h>

namespace ila::mc::inline actor::inline boss
{

void WitherDestroyBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["level"]  = serializeRefObj(mLevel);
    nbt["box"]    = { { "min", ListTag { mBox.min.x, mBox.min.y, mBox.min.z } },
                      { "max", ListTag { mBox.max.x, mBox.max.y, mBox.max.z } } };
    nbt["radius"] = mRadius;
    nbt["dimId"]  = getDimensionName(blockSource());
}
void WitherDestroyBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mBox.min.x = nbt["box"]["min"][0];
    mBox.min.y = nbt["box"]["min"][1];
    mBox.min.z = nbt["box"]["min"][2];
    mBox.max.x = nbt["box"]["max"][0];
    mBox.max.y = nbt["box"]["max"][1];
    mBox.max.z = nbt["box"]["max"][2];
    mRadius    = nbt["radius"];
};;

void WitherDestroyAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["level"]  = serializeRefObj(mLevel);
    nbt["box"]    = { { "min", ListTag { mBox.min.x, mBox.min.y, mBox.min.z } },
                      { "max", ListTag { mBox.max.x, mBox.max.y, mBox.max.z } } };
    nbt["radius"] = mRadius;
    nbt["dimId"]  = getDimensionName(blockSource());
};;

LL_TYPE_INSTANCE_HOOK(
    WitherDestroyEventHook,
    HookPriority::Normal,
    WitherBoss,
    &WitherBoss::_destroyBlocks,
    void,
    Level&                       pLevel,
    AABB const&                  pBox,
    BlockSource&                 pRegion,
    int                          pRange,
    WitherBoss::WitherAttackType pType
)
{
    auto beforeEvent = WitherDestroyBeforeEvent(pRegion, pLevel, const_cast<AABB&>(pBox), pRange);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pLevel, pBox, pRegion, pRange, pType);
    LLEventBus.publish(WitherDestroyAfterEvent(pRegion, pLevel, pBox, pRange));
}

Event_Hook_Factory(WitherDestroy, <WitherDestroyEventHook>);

} // namespace ila::mc::inline actor::inline boss
