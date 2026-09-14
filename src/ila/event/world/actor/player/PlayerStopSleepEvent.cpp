#include "ila/event/world/actor/player/PlayerStopSleepEvent.h"
#include "ila/base/Gloabl.h"

namespace ila::mc::inline world::inline actor::inline player {

void PlayerStopSleepBeforeEvent::serialize(CompoundTag& nbt) const {
    PlayerEvent::serialize(nbt);
    nbt["forcefulWakeUp"]  = forcefulWakeUp();
    nbt["updateLevelList"] = updateLevelList();
}
void PlayerStopSleepBeforeEvent::deserialize(CompoundTag const& nbt) {
    PlayerEvent::deserialize(nbt);
    forcefulWakeUp()  = nbt["forcefulWakeUp"];
    updateLevelList() = nbt["updateLevelList"];
}
bool& PlayerStopSleepBeforeEvent::forcefulWakeUp() const { return mForcefulWakeUp; }
bool& PlayerStopSleepBeforeEvent::updateLevelList() const { return mUpdateLevelList; }

void PlayerStopSleepAfterEvent::serialize(CompoundTag& nbt) const {
    PlayerEvent::serialize(nbt);
    nbt["forcefulWakeUp"]  = forcefulWakeUp();
    nbt["updateLevelList"] = updateLevelList();
}
bool const& PlayerStopSleepAfterEvent::forcefulWakeUp() const { return mForcefulWakeUp; }
bool const& PlayerStopSleepAfterEvent::updateLevelList() const { return mUpdateLevelList; }

LL_TYPE_INSTANCE_HOOK(
    PlayerStopSleepEventHook,
    HookPriority::Normal,
    Player,
    &Player::$stopSleepInBed,
    void,
    bool forcefulWakeUp,
    bool updateLevelList
) {
    LLEventBus.publish(PlayerStopSleepBeforeEvent(*this, forcefulWakeUp, updateLevelList));
    origin(forcefulWakeUp, updateLevelList);
    LLEventBus.publish(PlayerStopSleepAfterEvent(*this, forcefulWakeUp, updateLevelList));
}

Event_Hook_Factory(PlayerStopSleep, <PlayerStopSleepEventHook>);

} // namespace ila::mc::inline world::inline actor::inline player
