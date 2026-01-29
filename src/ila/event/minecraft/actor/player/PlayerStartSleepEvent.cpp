#include "ila/event/minecraft/actor/player/PlayerStartSleepEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/player/PlayerEvent.h>
#include <ll/api/memory/Hook.h>
#include <magic_enum.hpp>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/nbt/StringTag.h>
#include <mc/world/actor/player/BedSleepingResult.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/BlockPos.h>

namespace ila::mc::inline actor::inline player
{

void PlayerStartSleepBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"] = ListTag { pos().x, pos().y, pos().z };
}
void PlayerStartSleepBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x = nbt["pos"][0];
    pos().y = nbt["pos"][1];
    pos().z = nbt["pos"][2];
}
BlockPos& PlayerStartSleepBeforeEvent::pos() const { return mPos; }

void PlayerStartSleepAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["pos"]    = ListTag { pos().x, pos().y, pos().z };
    nbt["result"] = magic_enum::enum_name(result());
}
void PlayerStartSleepAfterEvent::deserialize(CompoundTag const& nbt)
{
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
    BlockPos const& bedBlockPos
)
{
    auto beforeEvent = PlayerStartSleepBeforeEvent(*this, const_cast<BlockPos&>(bedBlockPos));
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return BedSleepingResult::OtherProblem; }
    auto result = origin(bedBlockPos);
    LLEventBus.publish(PlayerStartSleepAfterEvent(*this, bedBlockPos, result));
    return result;
}

Event_Hook_Factory(PlayerStartSleep, <PlayerStartSleepEventHook>);

} // namespace ila::mc::inline actor::inline player