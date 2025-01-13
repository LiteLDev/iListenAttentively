#include "ila/event/minecraft/world/actor/ActorPickupItemEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/actor/ai/goal/PickupItemsGoal.h>

namespace ila::mc::inline actor
{

void ActorPickupItemBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["itemActor"] = serializeRefObj(getItemActor());
}
ItemActor& ActorPickupItemBeforeEvent::getItemActor() const { return mItemActor; };

void ActorPickupItemAfterEvent::serialize(CompoundTag& nbt) const
{
    MobEvent::serialize(nbt);
    nbt["itemActor"] = serializeRefObj(getItemActor());
}
ItemActor const& ActorPickupItemAfterEvent::getItemActor() const { return mItemActor; };

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