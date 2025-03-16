#include "ila/event/minecraft/world/WitherDestroyEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/actor/boss/WitherBoss.h>

namespace ila::mc::inline world
{

void WitherDestroyBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["level"]  = serializeRefObj(level());
    nbt["box"]    = { { "min", ListTag { box().min.x, box().min.y, box().min.z } },
                      { "max", ListTag { box().max.x, box().max.y, box().max.z } } };
    nbt["radius"] = radius();
    nbt["dimId"]  = getDimensionName(blockSource());
}
void WitherDestroyBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    box().min.x = nbt["box"]["min"][0];
    box().min.y = nbt["box"]["min"][1];
    box().min.z = nbt["box"]["min"][2];
    box().max.x = nbt["box"]["max"][0];
    box().max.y = nbt["box"]["max"][1];
    box().max.z = nbt["box"]["max"][2];
    radius()    = nbt["radius"];
}
Level& WitherDestroyBeforeEvent::level() const { return mLevel; }
AABB&  WitherDestroyBeforeEvent::box() const { return mBox; };
int&   WitherDestroyBeforeEvent::radius() const { return mRadius; };

void WitherDestroyAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["level"]  = serializeRefObj(level());
    nbt["box"]    = { { "min", ListTag { box().min.x, box().min.y, box().min.z } },
                      { "max", ListTag { box().max.x, box().max.y, box().max.z } } };
    nbt["radius"] = radius();
    nbt["dimId"]  = getDimensionName(blockSource());
}
Level&      WitherDestroyAfterEvent::level() const { return mLevel; }
AABB const& WitherDestroyAfterEvent::box() const { return mBox; };
int const&  WitherDestroyAfterEvent::radius() const { return mRadius; };

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

} // namespace ila::mc::inline world