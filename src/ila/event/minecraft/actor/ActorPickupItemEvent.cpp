#include "ila/event/minecraft/actor/ActorPickupItemEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/entity/MobEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ai/goal/PickupItemsGoal.h>

namespace ila::mc::inline actor
{

void ActorPickupItemBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["itemActor"] = serializeRefObj(mItemActor);
};

void ActorPickupItemAfterEvent::serialize(CompoundTag& nbt) const
{
    MobEvent::serialize(nbt);
    nbt["itemActor"] = serializeRefObj(mItemActor);
};

LL_TYPE_INSTANCE_HOOK(
    ActorPickupItemEventHook,
    HookPriority::Normal,
    PickupItemsGoal,
    &PickupItemsGoal::_pickItemUp,
    void,
    ItemActor* pItem
)
{
    if (pItem == nullptr) { return origin(pItem); }
    auto beforeEvent = ActorPickupItemBeforeEvent(mMob, *pItem);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pItem);
    LLEventBus.publish(ActorPickupItemAfterEvent(mMob, *pItem));
}

Event_Hook_Factory(ActorPickupItem, <ActorPickupItemEventHook>);

} // namespace ila::mc::inline actor