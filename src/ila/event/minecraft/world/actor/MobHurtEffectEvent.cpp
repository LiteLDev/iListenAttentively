#include "ila/event/minecraft/world/actor/MobHurtEffectEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/service/Bedrock.h>
#include <mc/deps/ecs/gamerefs_entity/GameRefsEntity.h>
#include <mc/entity/components/ActorOwnerComponent.h>
#include <mc/entity/components_json_legacy/SplashPotionEffectSubcomponent.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/world/actor/ActorDamageSource.h>
#include <mc/world/attribute/AttributeBuff.h>
#include <mc/world/attribute/HealthAttributeDelegate.h>
#include <mc/world/effect/EffectDuration.h>
#include <mc/world/level/Level.h>

namespace ila::mc::inline world::inline actor
{

void MobHurtEffectBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["source"] = serializePtrObj(source().as_ptr());
    nbt["value"]  = value();
    nbt["cause"]  = magic_enum::enum_name(cause());
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
    nbt["source"] = serializePtrObj(source().as_ptr());
    nbt["value"]  = value();
    nbt["cause"]  = magic_enum::enum_name(cause());
}
optional_ref<Actor const>                    MobHurtEffectAfterEvent::source() const { return mSource; }
float const&                                 MobHurtEffectAfterEvent::value() const { return mValue; }
SharedTypes::Legacy::ActorDamageCause const& MobHurtEffectAfterEvent::cause() const { return mCause; }

LL_TYPE_INSTANCE_HOOK(
    MobHurtEffectHook,
    HookPriority::Normal,
    HealthAttributeDelegate,
    &HealthAttributeDelegate::$getBuffValueWithModifiers,
    float,
    AttributeBuff const& buff
)
{
    if (buff.mSource->mCause == SharedTypes::Legacy::ActorDamageCause::Magic
        || buff.mSource->mCause == SharedTypes::Legacy::ActorDamageCause::Wither)
    {
        optional_ref<Actor> damageSource = std::nullopt;

        if (buff.mSource->isEntitySource())
        {
            damageSource = ll::service::getLevel()->fetchEntity(
                buff.mSource->isChildEntitySource() ? buff.mSource->getEntityUniqueID()
                                                    : buff.mSource->getDamagingEntityUniqueID(),
                false
            );
        }

        auto& newBuff = const_cast<AttributeBuff&>(buff);

        auto beforeEvent = MobHurtEffectBeforeEvent(
            *mMob,
            damageSource,
            newBuff.mAmount,
            const_cast<SharedTypes::Legacy::ActorDamageCause&>(buff.mSource->mCause)
        );
        LLEventBus.publish(beforeEvent);
        if (beforeEvent.isCancelled()) { return 0.0f; }
        LLEventBus.publish(MobHurtEffectAfterEvent(*mMob, damageSource, buff.mAmount, buff.mSource->mCause));
        return origin(newBuff);
    }
    return origin(buff);
}

Event_Hook_Factory(MobHurtEffect, <MobHurtEffectHook>);

} // namespace ila::mc::inline world::inline actor
