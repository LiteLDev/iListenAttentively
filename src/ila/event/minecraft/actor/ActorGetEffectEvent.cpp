#include "ila/event/minecraft/actor/ActorGetEffectEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/entity/ActorEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/effect/EffectDuration.h>
#include <mc/world/effect/MobEffectInstance.h>

namespace ila::mc::inline actor
{

void ActorGetEffectBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["effect"] = *mEffect.save();
}
void ActorGetEffectBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mEffect = MobEffectInstance::load(nbt["effect"].get<CompoundTag>());
};

void ActorGetEffectAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["effect"] = *mEffect.save();
};

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

} // namespace ila::mc::inline actor