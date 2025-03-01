#include "ila/event/minecraft/world/actor/player/PlayerAteEvent.h"
#include "ila/base/Gloabl.h"
#include "mc/world/item/BucketItem.h"
#include "mc/world/item/MedicineItem.h"
#include "mc/world/item/PotionItem.h"


namespace ila::mc::inline player
{

void PlayerAteEvent::serialize(CompoundTag& nbt) const
{
    ll::event::player::PlayerEvent::serialize(nbt);
    nbt["item"] = ll::event::serializeRefObj(mItem);
}

void PlayerAteEvent::deserialize(CompoundTag const& nbt) { ll::event::player::PlayerEvent::deserialize(nbt); }

ItemStack& PlayerAteEvent::getItem() const { return mItem; }

LL_TYPE_INSTANCE_HOOK(
    PlayerAteEventHook1,
    HookPriority::Normal,
    Player,
    &Player::eat,
    void,
    ItemStack const& instance
)
{
    LLEventBus.publish(PlayerAteEvent { *this, const_cast<ItemStack&>(instance) });
}

LL_TYPE_INSTANCE_HOOK(
    PlayerAteEventHook2,
    HookPriority::Normal,
    PotionItem,
    &PotionItem::$useTimeDepleted,
    ItemUseMethod,
    ItemStack& inoutInstance,
    Level*     level,
    Player*    player
)
{
    LLEventBus.publish(PlayerAteEvent { *player, inoutInstance });
    return origin(inoutInstance, level, player);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerAteEventHook3,
    HookPriority::Normal,
    PotionItem,
    (uintptr_t)BucketItem::$vftable()[79],
    ItemUseMethod,
    ItemStack& inoutInstance,
    Level*     level,
    Player*    player
)
{
    LLEventBus.publish(PlayerAteEvent { *player, inoutInstance });
    return origin(inoutInstance, level, player);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerAteEventHook4,
    HookPriority::Normal,
    MedicineItem,
    &MedicineItem::$useTimeDepleted,
    ItemUseMethod,
    ItemStack& inoutInstance,
    Level*     level,
    Player*    player
)
{
    LLEventBus.publish(PlayerAteEvent { *player, inoutInstance });
    return origin(inoutInstance, level, player);
}

Event_Listener_Factory(PlayerAte)
{
    ll::memory::
        HookRegistrar<PlayerAteEventHook1, PlayerAteEventHook2, PlayerAteEventHook3, PlayerAteEventHook4>
            hook;
}

} // namespace ila::mc::inline player