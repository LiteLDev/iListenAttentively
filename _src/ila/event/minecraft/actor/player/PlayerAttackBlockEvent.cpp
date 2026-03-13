#include "ila/event/actor/player/PlayerAttackBlockEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/player/PlayerEvent.h>
#include <ll/api/memory/Hook.h>
#include <magic_enum.hpp>
#include <mc/common/FacingID.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/nbt/StringTag.h>
#include <mc/network/ServerPlayerBlockUseHandler.h>
#include <mc/server/ServerPlayer.h>
#include <mc/world/level/BlockPos.h>

namespace ila::mc::inline actor::inline player
{

void PlayerAttackBlockBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]  = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["face"] = magic_enum::enum_name(mFace);
}
void PlayerAttackBlockBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mPos.x = nbt["pos"][0];
    mPos.y = nbt["pos"][1];
    mPos.z = nbt["pos"][2];
    mFace  = magic_enum::enum_cast<FacingID>(nbt["face"].get<StringTag>()).value_or(mFace);
}

void PlayerAttackBlockAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["pos"]  = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["face"] = magic_enum::enum_name(mFace);
}

LL_STATIC_HOOK(
    PlayerAttackBlockEventHook,
    HookPriority::Normal,
    &ServerPlayerBlockUseHandler::onStartDestroyBlock,
    void,
    ServerPlayer&   player,
    BlockPos const& pos,
    int             face
)
{
    auto beforeEvent =
        PlayerAttackBlockBeforeEvent(player, const_cast<BlockPos&>(pos), *reinterpret_cast<FacingID*>(&face));
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(player, pos, face);
    LLEventBus.publish(PlayerAttackBlockAfterEvent(player, pos, *reinterpret_cast<FacingID*>(&face)));
}

Event_Hook_Factory(PlayerAttackBlock, <PlayerAttackBlockEventHook>);

} // namespace ila::mc::inline actor::inline player