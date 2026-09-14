#include "ila/event/world/actor/ActorPickupItemEvent.h"
#include "ila/base/Gloabl.h"
#include "mc/world/actor/ActorType.h"
#include <mc/gameplayhandlers/ActorGameplayHandler.h>
#include <mc/world/actor/ai/goal/PickupItemsGoal.h>
#include <mc/world/actor/item/ItemActor.h>
#include <mc/world/events/ActorEventListener.h>
#include <mc/world/events/ActorGameplayEvent.h>
#include <mc/world/events/EventCoordinatorPimpl.h>

namespace ila::mc::inline world::inline actor {

void ActorPickupItemBeforeEvent::serialize(CompoundTag& nbt) const {
    Cancellable::serialize(nbt);
    nbt["itemActor"] = serializeRefObj(itemActor());
}
ItemActor& ActorPickupItemBeforeEvent::itemActor() const { return mItemActor; };

void ActorPickupItemAfterEvent::serialize(CompoundTag& nbt) const {
    ActorEvent::serialize(nbt);
    nbt["itemActor"] = serializeRefObj(itemActor());
}
ItemActor const& ActorPickupItemAfterEvent::itemActor() const { return mItemActor; };

LL_TYPE_INSTANCE_HOOK(
    ActorPickupItemEventHook,
    HookPriority::Normal,
    EventCoordinatorPimpl<ActorEventListener>,
    &EventCoordinatorPimpl<ActorEventListener>::_processEvent,
    CoordinatorResult,
    ActorGameplayHandler*                         handler,
    MutableActorGameplayEvent<CoordinatorResult>& event
)
try {
    return event.visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Details::ValueOrRef<ActorBeforeAcquireItemEvent const>>) {
            ActorBeforeAcquireItemEvent const& acruireEvent = arg.value();
            if (acruireEvent.mItem.isType(ActorType::ItemEntity)) {
                auto beforeEvent =
                    ActorPickupItemBeforeEvent(acruireEvent.mActor, static_cast<ItemActor&>(acruireEvent.mItem));
                LLEventBus.publish(beforeEvent);
                if (beforeEvent.isCancelled()) {
                    origin(handler, event);
                    return CoordinatorResult::Cancel;
                }
                LLEventBus.publish(
                    ActorPickupItemAfterEvent(acruireEvent.mActor, static_cast<ItemActor&>(acruireEvent.mItem))
                );
                LLEventBus.publish(beforeEvent);
            }
        }
        return origin(handler, event);
    });
} catch (...) {
    return origin(handler, event);
}

Event_Hook_Factory(ActorPickupItem, <ActorPickupItemEventHook>);

} // namespace ila::mc::inline world::inline actor
