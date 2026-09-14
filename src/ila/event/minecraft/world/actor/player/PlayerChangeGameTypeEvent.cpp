#include "ila/event/minecraft/world/actor/player/PlayerChangeGameTypeEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/network/packet/SetPlayerGameTypePacket.h>

namespace ila::mc::inline world::inline actor::inline player {

void PlayerChangeGameTypeBeforeEvent::serialize(CompoundTag& nbt) const {
    Cancellable::serialize(nbt);
    nbt["oldGameType"] = magic_enum::enum_name(oldGameType());
    nbt["newGameType"] = magic_enum::enum_name(newGameType());
}
void PlayerChangeGameTypeBeforeEvent::deserialize(CompoundTag const& nbt) {
    Cancellable::deserialize(nbt);
    newGameType() = magic_enum::enum_cast<GameType>(nbt["newGameType"].get<StringTag>()).value_or(newGameType());
}
GameType const& PlayerChangeGameTypeBeforeEvent::oldGameType() const { return mOldGameType; }
GameType&       PlayerChangeGameTypeBeforeEvent::newGameType() const { return mNewGameType; }

void PlayerChangeGameTypeAfterEvent::serialize(CompoundTag& nbt) const {
    ServerPlayerEvent::serialize(nbt);
    nbt["oldGameType"] = magic_enum::enum_name(oldGameType());
    nbt["newGameType"] = magic_enum::enum_name(newGameType());
}
GameType const& PlayerChangeGameTypeAfterEvent::oldGameType() const { return mOldGameType; }
GameType const& PlayerChangeGameTypeAfterEvent::newGameType() const { return mNewGameType; }

LL_TYPE_INSTANCE_HOOK(
    PlayerChangeGameTypeEventHook,
    HookPriority::Normal,
    ServerPlayer,
    &ServerPlayer::$setPlayerGameType,
    void,
    GameType pNewGameType
) {
    auto const oldGameType = getPlayerGameType();
    if (oldGameType == pNewGameType) {
        return origin(pNewGameType);
    }
    auto beforeEvent = PlayerChangeGameTypeBeforeEvent(*this, oldGameType, pNewGameType);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) {
        return;
    }
    origin(pNewGameType);
    if (pNewGameType == getPlayerGameType()) {
        LLEventBus.publish(PlayerChangeGameTypeAfterEvent(*this, oldGameType, pNewGameType));
    }
}

Event_Hook_Factory(PlayerChangeGameType, <PlayerChangeGameTypeEventHook>);

} // namespace ila::mc::inline world::inline actor::inline player