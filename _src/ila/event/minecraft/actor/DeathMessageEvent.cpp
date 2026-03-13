#include "ila/event/actor/DeathMessageEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/entity/ActorEvent.h>
#include <ll/api/memory/Hook.h>
#include <magic_enum.hpp>
#include <mc/deps/shared_types/legacy/actor/ActorDamageCause.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/CompoundTagVariant.h>
#include <mc/nbt/ListTag.h>
#include <mc/nbt/StringTag.h>
#include <mc/world/actor/ActorDamageByActorSource.h>
#include <mc/world/actor/ActorDamageByBlockSource.h>
#include <mc/world/actor/ActorDamageByChildActorSource.h>
#include <mc/world/actor/ActorDamageSource.h>
#include <string>
#include <utility>
#include <vector>

namespace ila::mc::inline actor
{

using DEATH_MESSAGE = std::pair<std::string, std::vector<std::string>>;

void DeathMessageBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["sourceUniqueId"] = mDamageSource.getEntityUniqueID().rawID;
    nbt["cause"]          = magic_enum::enum_name(mDamageSource.mCause);
    nbt["result"]         = { { "key", mResult.first }, { "params", ListTag {} } };
    for (auto& param : mResult.second) { nbt["result"]["params"].push_back(param); }
}
void DeathMessageBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mResult.first = nbt["result"]["key"];
    mDamageSource.mCause =
        magic_enum::enum_cast<SharedTypes::Legacy::ActorDamageCause>(nbt["cause"].get<StringTag>())
            .value_or(mDamageSource.mCause);
    for (auto& param : nbt["result"]["params"].get<ListTag>()) { mResult.second.push_back(param); }
}

void DeathMessageAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["sourceUniqueId"] = mDamageSource.getEntityUniqueID().rawID;
    nbt["cause"]          = magic_enum::enum_name(mDamageSource.mCause);
    nbt["result"]         = { { "key", mResult.first }, { "params", ListTag {} } };
    for (auto& param : mResult.second) { nbt["result"]["params"].push_back(param); }
}

#define DeathMessageHookMacro(name, type)                                                                    \
    LL_TYPE_INSTANCE_HOOK(                                                                                   \
        name,                                                                                                \
        HookPriority::Normal,                                                                                \
        type,                                                                                                \
        &type::$getDeathMessage,                                                                             \
        DEATH_MESSAGE,                                                                                       \
        std::string pDeadName,                                                                               \
        Actor*      pDeadActor                                                                               \
    )                                                                                                        \
    {                                                                                                        \
        auto result = origin(pDeadName, pDeadActor);                                                         \
        if (pDeadActor == nullptr || result.first.empty()) { return result; }                                \
        auto beforeEvent = DeathMessageBeforeEvent(*pDeadActor, *this, result);                              \
        LLEventBus.publish(beforeEvent);                                                                     \
        if (beforeEvent.isCancelled()) { return DEATH_MESSAGE(); }                                           \
        LLEventBus.publish(DeathMessageAfterEvent(*pDeadActor, *this, result));                              \
        return result;                                                                                       \
    }

DeathMessageHookMacro(DeathMessageEventHook1, ActorDamageSource);

DeathMessageHookMacro(DeathMessageEventHook2, ActorDamageByActorSource);

DeathMessageHookMacro(DeathMessageEventHook3, ActorDamageByBlockSource);

DeathMessageHookMacro(DeathMessageEventHook4, ActorDamageByChildActorSource);

Event_Hook_Factory(
    DeathMessage,
    <DeathMessageEventHook1, DeathMessageEventHook2, DeathMessageEventHook3, DeathMessageEventHook4>
);

} // namespace ila::mc::inline actor