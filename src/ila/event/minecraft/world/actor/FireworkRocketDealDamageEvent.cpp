#include "ila/event/minecraft/world/actor/FireworkRocketDealDamageEvent.h"
#include "ila/base/Gloabl.h"

namespace ila::mc::inline world::inline actor
{

void FireworkRocketDealDamageBeforeEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["self"] = serializeRefObj(self());
}
FireworksRocketActor& FireworkRocketDealDamageBeforeEvent::self() const
{
    return static_cast<FireworksRocketActor&>(ActorEvent::self());
}

void FireworkRocketDealDamageAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["self"] = serializeRefObj(self());
}
FireworksRocketActor& FireworkRocketDealDamageAfterEvent::self() const
{
    return static_cast<FireworksRocketActor&>(ActorEvent::self());
}

LL_TYPE_INSTANCE_HOOK(
    FireworkRocketDealDamageEventHook,
    HookPriority::Normal,
    FireworksRocketActor,
    &FireworksRocketActor::dealExplosionDamage,
    void
)
{
    auto beforeEvent = FireworkRocketDealDamageBeforeEvent(*this);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin();
    LLEventBus.publish(FireworkRocketDealDamageAfterEvent(*this));
}

Event_Hook_Factory(FireworkRocketDealDamage, <FireworkRocketDealDamageEventHook>);

} // namespace ila::mc::inline world::inline actor