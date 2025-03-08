#include "ila/event/minecraft/world/actor/ActorRideEvent.h"
#include "ila/base/Gloabl.h"

namespace ila::mc::inline world::inline actor
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

} // namespace ila::mc::inline world::inline actor