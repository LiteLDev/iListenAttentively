#include "ila/event/minecraft/world/actor/player/PlayerAteEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/item/BucketItem.h>
#include <mc/world/item/MedicineItem.h>
#include <mc/world/item/PotionItem.h>

namespace ila::mc::inline world::inline actor::inline player
{

void PlayerAteEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["item"] = serializeRefObj(mItem);
}

void PlayerAteEvent::deserialize(CompoundTag const& nbt) { PlayerEvent::deserialize(nbt); }

ItemStack& PlayerAteEvent::getItem() const { return mItem; }

LL_TYPE_INSTANCE_HOOK(
    PlayerAteEventHook1,
    HookPriority::Normal,
    Player,
    &Player::eat,
    void,
    ItemStack const& pInstance
)
{
    LLEventBus.publish(PlayerAteEvent(*this, const_cast<ItemStack&>(pInstance)));
    return origin(pInstance);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerAteEventHook2,
    HookPriority::Normal,
    PotionItem,
    &PotionItem::$useTimeDepleted,
    ItemUseMethod,
    ItemStack& pInoutInstance,
    Level*     pLevel,
    Player*    pPlayer
)
{
    if (pPlayer) { LLEventBus.publish(PlayerAteEvent(*pPlayer, pInoutInstance)); }
    return origin(pInoutInstance, pLevel, pPlayer);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerAteEventHook3,
    HookPriority::Normal,
    PotionItem,
    reinterpret_cast<uintptr_t>(BucketItem::$vftable()[79]),
    ItemUseMethod,
    ItemStack& pInoutInstance,
    Level*     pLevel,
    Player*    pPlayer
)
{
    if (pPlayer) { LLEventBus.publish(PlayerAteEvent { *pPlayer, pInoutInstance }); }
    return origin(pInoutInstance, pLevel, pPlayer);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerAteEventHook4,
    HookPriority::Normal,
    MedicineItem,
    &MedicineItem::$useTimeDepleted,
    ItemUseMethod,
    ItemStack& pInoutInstance,
    Level*     pLevel,
    Player*    pPlayer
)
{
    if (pPlayer) { LLEventBus.publish(PlayerAteEvent { *pPlayer, pInoutInstance }); }
    return origin(pInoutInstance, pLevel, pPlayer);
}

Event_Hook_Factory_Base(PlayerAte, <PlayerAteEventHook1, PlayerAteEventHook2, PlayerAteEventHook3, PlayerAteEventHook4>)

} // namespace ila::mc::inline player