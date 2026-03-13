#include "ila/event/actor/FireworkRocketDealDamageEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/entity/ActorEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/item/FireworksRocketActor.h>

namespace ila::mc::inline actor
{

void FireworkRocketDealDamageBeforeEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["self"] = serializeRefObj(self());
}

void FireworkRocketDealDamageAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["self"] = serializeRefObj(self());
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

} // namespace ila::mc::inline actor
