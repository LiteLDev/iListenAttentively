#include "ila/event/minecraft/actor/ActorRideEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/entity/ActorEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/Actor.h>

namespace ila::mc::inline actor
{

void ActorRideBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["target"] = serializeRefObj(target());
}
Actor& ActorRideBeforeEvent::target() const { return mTarget; }

void ActorRideAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["target"] = serializeRefObj(target());
}
Actor const& ActorRideAfterEvent::target() const { return mTarget; }

LL_TYPE_INSTANCE_HOOK(
    ActorRideEventHook,
    HookPriority::Normal,
    Actor,
    &Actor::$addPassenger,
    void,
    Actor& pPassenger
)
{
    if (!canAddPassenger(pPassenger)) { return origin(pPassenger); }
    auto beforeEvent = ActorRideBeforeEvent(pPassenger, *this);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pPassenger);
    if (isPassenger(pPassenger)) { LLEventBus.publish(ActorRideAfterEvent(pPassenger, *this)); }
}

Event_Hook_Factory(ActorRide, <ActorRideEventHook>);

} // namespace ila::mc::inline actor