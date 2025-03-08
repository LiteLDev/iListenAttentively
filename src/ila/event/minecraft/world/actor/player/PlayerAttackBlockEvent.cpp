#include "ila/event/minecraft/world/actor/player/PlayerAttackBlockEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/network/ServerPlayerBlockUseHandler.h>

namespace ila::mc::inline world::inline actor::inline player
{

void PlayerAttackBlockBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]  = ListTag { pos().x, pos().y, pos().z };
    nbt["face"] = magic_enum::enum_name(face());
}
void PlayerAttackBlockBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x = nbt["pos"][0];
    pos().y = nbt["pos"][1];
    pos().z = nbt["pos"][2];
    face()  = magic_enum::enum_cast<FacingID>(nbt["face"].get<StringTag>()).value_or(face());
}
BlockPos& PlayerAttackBlockBeforeEvent::pos() const { return mPos; }
FacingID& PlayerAttackBlockBeforeEvent::face() const { return mFace; }

void PlayerAttackBlockAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["pos"]  = ListTag { pos().x, pos().y, pos().z };
    nbt["face"] = magic_enum::enum_name(face());
}
BlockPos const& PlayerAttackBlockAfterEvent::pos() const { return mPos; }
FacingID const& PlayerAttackBlockAfterEvent::face() const { return mFace; }

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

} // namespace ila::mc::inline world::inline actor::inline player