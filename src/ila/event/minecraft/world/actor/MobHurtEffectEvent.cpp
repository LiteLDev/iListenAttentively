#include "ila/event/minecraft/world/actor/MobHurtEffectEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/service/Bedrock.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/world/actor/ActorDamageSource.h>
#include <mc/world/effect/EffectDuration.h>
#include <mc/world/level/Level.h>

namespace ila::mc::inline world::inline actor
{

void MobHurtEffectBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    if (source().has_value()) { nbt["source"] = serializeRefObj(*source()); }
    nbt["value"] = value();
    nbt["cause"] = magic_enum::enum_name(cause());
}
void MobHurtEffectBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    value() = nbt["value"];
    cause() = magic_enum::enum_cast<SharedTypes::Legacy::ActorDamageCause>(nbt["cause"].get<StringTag>())
                     .value_or(cause());
}
optional_ref<Actor>                    MobHurtEffectBeforeEvent::source() const { return mSource; }
float&                                 MobHurtEffectBeforeEvent::value() const { return mValue; }
SharedTypes::Legacy::ActorDamageCause& MobHurtEffectBeforeEvent::cause() const { return mCause; }

void MobHurtEffectAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    if (source().has_value()) { nbt["source"] = serializeRefObj(*source()); }
    nbt["value"] = value();
    nbt["cause"] = magic_enum::enum_name(cause());
}
optional_ref<Actor const>                    MobHurtEffectAfterEvent::source() const { return mSource; }
float const&                                 MobHurtEffectAfterEvent::value() const { return mValue; }
SharedTypes::Legacy::ActorDamageCause const& MobHurtEffectAfterEvent::cause() const { return mCause; }

LL_TYPE_INSTANCE_HOOK(
    MobHurtEffectHook,
    HookPriority::Normal,
    Mob,
    &Mob::getDamageAfterResistanceEffect,
    float,
    ActorDamageSource const& source,
    float                    damage
)
{
    if (source.mCause == SharedTypes::Legacy::ActorDamageCause::Magic
        || source.mCause == SharedTypes::Legacy::ActorDamageCause::Wither)
    {
        optional_ref<Actor> damageSource = std::nullopt;
        if (source.isEntitySource())
        {
            damageSource = ll::service::getLevel()->fetchEntity(
                source.isChildEntitySource() ? source.getEntityUniqueID()
                                             : source.getDamagingEntityUniqueID(),
                false
            );
        }
        auto beforeEvent = MobHurtEffectBeforeEvent(
            *this,
            damageSource,
            damage,
            const_cast<SharedTypes::Legacy::ActorDamageCause&>(source.mCause)
        );
        LLEventBus.publish(beforeEvent);
        if (beforeEvent.isCancelled()) { return 0.0f; }
        LLEventBus.publish(MobHurtEffectAfterEvent(*this, damageSource, damage, source.mCause));
    }
    return origin(source, damage);
}

Event_Hook_Factory(MobHurtEffect, <MobHurtEffectHook>);

} // namespace ila::mc::inline world::inline actor