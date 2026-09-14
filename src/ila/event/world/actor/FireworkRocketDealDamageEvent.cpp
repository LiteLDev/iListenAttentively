#include "ila/event/world/actor/FireworkRocketDealDamageEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorDamageByActorSource.h>
#include <mc/world/actor/ActorHurtResult.h>

namespace ila::mc::inline world::inline actor {

void FireworkRocketDealDamageBeforeEvent::serialize(CompoundTag& nbt) const {
    ActorEvent::serialize(nbt);
    nbt["self"] = serializeRefObj(self());
}
FireworksRocketActor& FireworkRocketDealDamageBeforeEvent::self() const {
    return static_cast<FireworksRocketActor&>(ActorEvent::self());
}

void FireworkRocketDealDamageAfterEvent::serialize(CompoundTag& nbt) const {
    ActorEvent::serialize(nbt);
    nbt["self"] = serializeRefObj(self());
}
FireworksRocketActor& FireworkRocketDealDamageAfterEvent::self() const {
    return static_cast<FireworksRocketActor&>(ActorEvent::self());
}

FireworksRocketActor* fireWorkActor = nullptr;

LL_TYPE_INSTANCE_HOOK(
    FireworkRocketDealDamageEventHook1,
    HookPriority::Normal,
    FireworksRocketActor,
    &FireworksRocketActor::postNormalTick,
    void
) {
    fireWorkActor = this;
    origin();
    fireWorkActor = nullptr;
}

LL_TYPE_INSTANCE_HOOK(
    FireworkRocketDealDamageEventHook2,
    HookPriority::Normal,
    Actor,
    &Actor::hurt,
    ActorHurtResult,
    ActorDamageSource const& source,
    float                    damage,
    HurtParameters const&    hurtParameters
) {
    if (fireWorkActor && source.isEntitySource()
        && source.getEntityUniqueID() == fireWorkActor->getOrCreateUniqueID()) {
        auto beforeEvent = FireworkRocketDealDamageBeforeEvent(*fireWorkActor);
        LLEventBus.publish(beforeEvent);
        if (beforeEvent.isCancelled()) {
            return ActorHurtResult{false, false};
        }
        return origin(source, damage, hurtParameters);
        LLEventBus.publish(FireworkRocketDealDamageAfterEvent(*fireWorkActor));
    }
    return origin(source, damage, hurtParameters);
}

Event_Hook_Factory(FireworkRocketDealDamage, <FireworkRocketDealDamageEventHook1, FireworkRocketDealDamageEventHook2>);

} // namespace ila::mc::inline world::inline actor
