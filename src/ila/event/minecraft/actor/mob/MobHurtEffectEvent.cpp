#include "ila/event/minecraft/actor/mob/MobHurtEffectEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/base/Containers.h>
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/entity/ActorEvent.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/service/Bedrock.h>
#include <magic_enum.hpp>
#include <mc/deps/core/utility/optional_ref.h>
#include <mc/deps/ecs/gamerefs_entity/EntityContext.h>
#include <mc/deps/ecs/gamerefs_entity/GameRefsEntity.h>
#include <mc/deps/game_refs/WeakRef.h>
#include <mc/deps/shared_types/legacy/actor/ActorDamageCause.h>
#include <mc/entity/components/ActorOwnerComponent.h>
#include <mc/entity/components_json_legacy/SplashPotionEffectSubcomponent.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/StringTag.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorDamageSource.h>
#include <mc/world/actor/Mob.h>
#include <mc/world/effect/EffectDuration.h>
#include <mc/world/level/Level.h>
#include <memory>
#include <optional>
#include <vector>

namespace ila::mc::inline actor::inline mob
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

static ll::DenseMap<Actor*, WeakRef<EntityContext>> mSplashPotionSources;

LL_TYPE_INSTANCE_HOOK(
    SplashPotionEffectSubcomponentApplyMobEffectsHook,
    HookPriority::Normal,
    SplashPotionEffectSubcomponent,
    &SplashPotionEffectSubcomponent::applyMobEffects,
    void,
    MobEffectInstance const&             effectInst,
    std::vector<Actor*> const&           actors,
    Actor&                               projectile,
    std::shared_ptr<Potion const> const& potion,
    float                                splashRange,
    float                                collisionMargin,
    MobEffect*                           effect,
    HitResult&                           res,
    int                                  aux,
    BaseGameVersion const&               currVer
)
{
    for (auto actor : actors) { mSplashPotionSources[actor] = projectile.getEntityContext().getWeakRef(); }
    origin(effectInst, actors, projectile, potion, splashRange, collisionMargin, effect, res, aux, currVer);
}

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
        else if (mSplashPotionSources.contains(this))
        {
            damageSource = mSplashPotionSources[this].tryUnwrap();
            mSplashPotionSources.erase(this);
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

Event_Hook_Factory(MobHurtEffect, <MobHurtEffectHook, SplashPotionEffectSubcomponentApplyMobEffectsHook>);

} // namespace ila::mc::inline actor::inline mob