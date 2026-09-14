#include "ila/event/world/actor/player/PlayerStartSleepEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/BlockPos.h>

namespace ila::mc::inline world::inline actor::inline player {

void PlayerStartSleepBeforeEvent::serialize(CompoundTag& nbt) const {
    Cancellable::serialize(nbt);
    nbt["pos"] = ListTag{pos().x, pos().y, pos().z};
}
void PlayerStartSleepBeforeEvent::deserialize(CompoundTag const& nbt) {
    Cancellable::deserialize(nbt);
    pos().x = nbt["pos"][0];
    pos().y = nbt["pos"][1];
    pos().z = nbt["pos"][2];
}
BlockPos& PlayerStartSleepBeforeEvent::pos() const { return mPos; }

void PlayerStartSleepAfterEvent::serialize(CompoundTag& nbt) const {
    PlayerEvent::serialize(nbt);
    nbt["pos"]    = ListTag{pos().x, pos().y, pos().z};
    nbt["result"] = magic_enum::enum_name(result());
}
void PlayerStartSleepAfterEvent::deserialize(CompoundTag const& nbt) {
    PlayerEvent::deserialize(nbt);
    result() = magic_enum::enum_cast<BedSleepingResult>(nbt["result"].get<StringTag>()).value_or(result());
}
BlockPos const&    PlayerStartSleepAfterEvent::pos() const { return mPos; }
BedSleepingResult& PlayerStartSleepAfterEvent::result() const { return mResult; }

LL_TYPE_INSTANCE_HOOK(
    PlayerStartSleepEventHook,
    HookPriority::Normal,
    Player,
    &Player::$startSleepInBed,
    BedSleepingResult,
    BlockPos const& bedPos,
    bool            setsRespawn,
    float           sleepOffset
) {
    auto beforeEvent = PlayerStartSleepBeforeEvent(*this, const_cast<BlockPos&>(bedPos));
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) {
        return BedSleepingResult::OtherProblem;
    }
    auto result = origin(bedPos, setsRespawn, sleepOffset);
    LLEventBus.publish(PlayerStartSleepAfterEvent(*this, bedPos, result));
    return result;
}

Event_Hook_Factory(PlayerStartSleep, <PlayerStartSleepEventHook>);

} // namespace ila::mc::inline world::inline actor::inline player
