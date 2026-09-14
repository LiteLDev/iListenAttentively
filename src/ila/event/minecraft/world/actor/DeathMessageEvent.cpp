#include "ila/event/minecraft/world/actor/DeathMessageEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/deps/nbt/CompoundTagVariant.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/world/actor/ActorDamageByActorSource.h>
#include <mc/world/actor/ActorDamageByBlockSource.h>
#include <mc/world/actor/ActorDamageByChildActorSource.h>


namespace ila::mc::inline world::inline actor {

using DEATH_MESSAGE = std::pair<std::string, std::vector<std::string>>;

void DeathMessageBeforeEvent::serialize(CompoundTag& nbt) const {
    Cancellable::serialize(nbt);
    nbt["sourceUniqueId"] = damageSource().getEntityUniqueID().rawID;
    nbt["cause"]          = magic_enum::enum_name(damageSource().mCause);
    nbt["result"]         = {
        {"key",    result().first},
        {"params", ListTag{}     }
    };
    for (auto& param : result().second) {
        nbt["result"]["params"].push_back(param);
    }
}
void DeathMessageBeforeEvent::deserialize(CompoundTag const& nbt) {
    Cancellable::deserialize(nbt);
    result().first        = nbt["result"]["key"];
    damageSource().mCause = magic_enum::enum_cast<SharedTypes::Legacy::ActorDamageCause>(nbt["cause"].get<StringTag>())
                                .value_or(damageSource().mCause);
    for (auto& param : nbt["result"]["params"].get<ListTag>()) {
        result().second.push_back(param);
    }
}
ActorDamageSource& DeathMessageBeforeEvent::damageSource() const { return mDamageSource; }
DEATH_MESSAGE&     DeathMessageBeforeEvent::result() const { return mResult; }

void DeathMessageAfterEvent::serialize(CompoundTag& nbt) const {
    ActorEvent::serialize(nbt);
    nbt["sourceUniqueId"] = damageSource().getEntityUniqueID().rawID;
    nbt["cause"]          = magic_enum::enum_name(damageSource().mCause);
    nbt["result"]         = {
        {"key",    result().first},
        {"params", ListTag{}     }
    };
    for (auto& param : result().second) {
        nbt["result"]["params"].push_back(param);
    }
}
ActorDamageSource const& DeathMessageAfterEvent::damageSource() const { return mDamageSource; }
DEATH_MESSAGE const&     DeathMessageAfterEvent::result() const { return mResult; }

#define DeathMessageHookMacro(name, type)                                                                              \
    LL_TYPE_INSTANCE_HOOK(                                                                                             \
        name,                                                                                                          \
        HookPriority::Normal,                                                                                          \
        type,                                                                                                          \
        &type::$_getDeathMessageInternal,                                                                              \
        DEATH_MESSAGE,                                                                                                 \
        std::string const& pDeadName,                                                                                  \
        Actor*             pDeadActor                                                                                  \
    ) {                                                                                                                \
        auto result = origin(pDeadName, pDeadActor);                                                                   \
        if (pDeadActor == nullptr || result.first.empty()) {                                                           \
            return result;                                                                                             \
        }                                                                                                              \
        auto beforeEvent = DeathMessageBeforeEvent(*pDeadActor, *this, result);                                        \
        LLEventBus.publish(beforeEvent);                                                                               \
        if (beforeEvent.isCancelled()) {                                                                               \
            return DEATH_MESSAGE();                                                                                    \
        }                                                                                                              \
        LLEventBus.publish(DeathMessageAfterEvent(*pDeadActor, *this, result));                                        \
        return result;                                                                                                 \
    }

DeathMessageHookMacro(DeathMessageEventHook1, ActorDamageSource);

DeathMessageHookMacro(DeathMessageEventHook2, ActorDamageByActorSource);

DeathMessageHookMacro(DeathMessageEventHook3, ActorDamageByBlockSource);

DeathMessageHookMacro(DeathMessageEventHook4, ActorDamageByChildActorSource);

Event_Hook_Factory(
    DeathMessage,
    <DeathMessageEventHook1, DeathMessageEventHook2, DeathMessageEventHook3, DeathMessageEventHook4>
);

} // namespace ila::mc::inline world::inline actor
