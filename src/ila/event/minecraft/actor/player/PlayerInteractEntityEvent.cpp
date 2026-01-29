#include "ila/event/minecraft/actor/player/PlayerInteractEntityEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/player/PlayerEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/player/Player.h>

namespace ila::mc::inline actor::inline player
{

void PlayerInteractEntityBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["target"] = serializeRefObj(target());
    nbt["pos"]    = ListTag { pos().x, pos().y, pos().z };
}
void PlayerInteractEntityBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x = nbt["pos"][0];
    pos().y = nbt["pos"][1];
    pos().z = nbt["pos"][2];
}
Actor& PlayerInteractEntityBeforeEvent::target() const { return mTarget; }
Vec3&  PlayerInteractEntityBeforeEvent::pos() const { return mPos; }

void PlayerInteractEntityAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["target"] = serializeRefObj(target());
    nbt["pos"]    = ListTag { pos().x, pos().y, pos().z };
}
Actor const& PlayerInteractEntityAfterEvent::target() const { return mTarget; }
Vec3 const&  PlayerInteractEntityAfterEvent::pos() const { return mPos; }

LL_TYPE_INSTANCE_HOOK(
    PlayerInteractEntityEventHook,
    HookPriority::Normal,
    Player,
    &Player::interact,
    bool,
    Actor&      pActor,
    Vec3 const& pLocation
)
{
    auto beforeEvent = PlayerInteractEntityBeforeEvent(*this, pActor, const_cast<Vec3&>(pLocation));
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return false; }
    auto result = origin(pActor, pLocation);
    if (result) { LLEventBus.publish(PlayerInteractEntityAfterEvent(*this, pActor, pLocation)); }
    return result;
}

Event_Hook_Factory(PlayerInteractEntity, <PlayerInteractEntityEventHook>);

} // namespace ila::mc::inline actor::inline player