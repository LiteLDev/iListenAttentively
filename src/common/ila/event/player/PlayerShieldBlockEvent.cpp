#include "ila/event/player/PlayerShieldBlockEvent.h"
#include "ila/base/Gloabl.i.h"
#include <ll/api/base/Containers.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/player/PlayerEvent.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/reflection/Serialization.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/player/Player.h>

namespace ila::player {

void PlayerShieldBlockEvent::serialize(CompoundTag& nbt) const {
    PlayerEvent::serialize(nbt);
    nbt["source"] = serializeRefObj(mSource);
    reflection::serialize_to(nbt["damage"], mDamage).value();
}

thread_local ll::SmallDenseSet<Player*> gBlockedPlayers;

LL_TYPE_INSTANCE_HOOK(
    PlayerShieldBlockEventHook1,
    HookPriority::Normal,
    Player,
    &Player::_blockUsingShield,
    bool,
    ActorDamageSource const& source,
    float                    damage
) {
    if (getCurrentActiveShield().isNull()) return origin(source, damage);

    if (eventPromise(PlayerShieldBlockingEvent{*this, source, damage}).publish()) return false;

    if (auto result = origin(source, damage); result) {
        gBlockedPlayers.insert(this);
        return true;
    }

    return false;
}

LL_TYPE_INSTANCE_HOOK(
    PlayerShieldBlockEventHook2,
    HookPriority::Normal,
    Player,
    &Player::$_hurt,
    bool,
    ActorDamageSource const& source,
    float                    damage,
    bool                     knock,
    bool                     ignite
) {
    gBlockedPlayers.erase(this);
    auto result = origin(source, damage, knock, ignite);
    if (gBlockedPlayers.contains(this) && !result) {
        eventPromise(PlayerShieldBlockedEvent{*this, source, damage}).publish();
    }
    gBlockedPlayers.erase(this);
    return result;
}

EventHook(
    PlayerShieldBlockingEvent,
    PlayerShieldBlockedEvent,
    <PlayerShieldBlockEventHook1, PlayerShieldBlockEventHook2>
);

} // namespace ila::player