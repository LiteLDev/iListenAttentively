#include "ila/event/minecraft/world/actor/player/PlayerAttackBlockEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/network/ServerPlayerBlockUseHandler.h>

namespace ila::mc::inline player
{

void PlayerAttackBlockBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]  = ListTag { getPos().x, getPos().y, getPos().z };
    nbt["face"] = magic_enum::enum_name(getFace());
}
void PlayerAttackBlockBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    getPos().x = nbt["pos"][0];
    getPos().y = nbt["pos"][1];
    getPos().z = nbt["pos"][2];
    getFace()  = magic_enum::enum_cast<FacingID>(nbt["face"].get<StringTag>()).value_or(getFace());
}
BlockPos& PlayerAttackBlockBeforeEvent::getPos() const { return mPos; }
FacingID& PlayerAttackBlockBeforeEvent::getFace() const { return mFace; }

void PlayerAttackBlockAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["pos"]  = ListTag { getPos().x, getPos().y, getPos().z };
    nbt["face"] = magic_enum::enum_name(getFace());
}
BlockPos const& PlayerAttackBlockAfterEvent::getPos() const { return mPos; }
FacingID const& PlayerAttackBlockAfterEvent::getFace() const { return mFace; }

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

} // namespace ila::mc::inline player