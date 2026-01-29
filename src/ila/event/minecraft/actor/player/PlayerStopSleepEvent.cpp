#include "ila/event/minecraft/actor/player/PlayerStopSleepEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/player/PlayerEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/player/Player.h>

namespace ila::mc::inline actor::inline player
{

void PlayerStopSleepBeforeEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["forcefulWakeUp"]  = mForcefulWakeUp;
    nbt["updateLevelList"] = mUpdateLevelList;
}
void PlayerStopSleepBeforeEvent::deserialize(CompoundTag const& nbt)
{
    PlayerEvent::deserialize(nbt);
    mForcefulWakeUp  = nbt["forcefulWakeUp"];
    mUpdateLevelList = nbt["updateLevelList"];
}

void PlayerStopSleepAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["forcefulWakeUp"]  = mForcefulWakeUp;
    nbt["updateLevelList"] = mUpdateLevelList;
}

LL_TYPE_INSTANCE_HOOK(
    PlayerStopSleepEventHook,
    HookPriority::Normal,
    Player,
    &Player::$stopSleepInBed,
    void,
    bool forcefulWakeUp,
    bool updateLevelList
)
{
    LLEventBus.publish(PlayerStopSleepBeforeEvent(*this, forcefulWakeUp, updateLevelList));
    origin(forcefulWakeUp, updateLevelList);
    LLEventBus.publish(PlayerStopSleepAfterEvent(*this, forcefulWakeUp, updateLevelList));
}

Event_Hook_Factory(PlayerStopSleep, <PlayerStopSleepEventHook>);

} // namespace ila::mc::inline actor::inline player