#include "ila/event/minecraft/world/actor/ActorGetEffectEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/effect/EffectDuration.h>
#include <mc/world/effect/MobEffectInstance.h>

namespace ila::mc::inline world::inline actor
{

void ActorGetEffectBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["effect"] = *effect().save();
}
void ActorGetEffectBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    effect() = MobEffectInstance::load(nbt["effect"].get<CompoundTag>());
}
MobEffectInstance& ActorGetEffectBeforeEvent::effect() const { return mEffect; };

void ActorGetEffectAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["effect"] = *effect().save();
}
MobEffectInstance const& ActorGetEffectAfterEvent::effect() const { return mEffect; };

LL_TYPE_INSTANCE_HOOK(
    ActorGetEffectEventHook,
    HookPriority::Normal,
    Actor,
    &Actor::addEffect,
    void,
    MobEffectInstance const& pEffect
)
{
    auto beforeEvent = ActorGetEffectBeforeEvent(*this, const_cast<MobEffectInstance&>(pEffect));
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pEffect);
    LLEventBus.publish(ActorGetEffectAfterEvent(*this, pEffect));
}

Event_Hook_Factory(ActorGetEffect, <ActorGetEffectEventHook>);

} // namespace ila::mc::inline world::inline actor