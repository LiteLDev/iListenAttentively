#include "ila/event/minecraft/actor/player/PlayerDropItemEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/player/PlayerEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/ContainerID.h>
#include <mc/world/actor/player/Inventory.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/actor/player/PlayerInventory.h>
#include <mc/world/inventory/transaction/ComplexInventoryTransaction.h>
#include <mc/world/inventory/transaction/InventoryAction.h>
#include <mc/world/inventory/transaction/InventorySource.h>
#include <mc/world/inventory/transaction/InventorySourceType.h>
#include <mc/world/inventory/transaction/InventoryTransaction.h>
#include <mc/world/inventory/transaction/InventoryTransactionError.h>
#include <mc/world/item/ItemStack.h>

namespace ila::mc::inline actor::inline player
{

void PlayerDropItemBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["item"] = serializeRefObj(item());
}
ItemStack const& PlayerDropItemBeforeEvent::item() const { return mItem; }

void PlayerDropItemAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["item"] = serializeRefObj(item());
}
ItemStack const& PlayerDropItemAfterEvent::item() const { return mItem; }

LL_TYPE_INSTANCE_HOOK(
    PlayerDropItemEventHook1,
    HookPriority::Normal,
    Player,
    &Player::$drop,
    bool,
    ItemStack const& pItem,
    bool             pRandomly
)
{
    auto beforeEvent = PlayerDropItemBeforeEvent(*this, const_cast<ItemStack&>(pItem));
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return false; }
    auto result = origin(pItem, pRandomly);
    if (result) { LLEventBus.publish(PlayerDropItemAfterEvent(*this, pItem)); }
    return result;
}

LL_TYPE_INSTANCE_HOOK(
    PlayerDropItemEventHook2,
    HookPriority::Normal,
    ComplexInventoryTransaction,
    &ComplexInventoryTransaction::$handle,
    InventoryTransactionError,
    Player& pPlayer,
    bool    pIsSenderAuthority
)
{
    if (mType != ComplexInventoryTransaction::Type::NormalTransaction)
    {
        return origin(pPlayer, pIsSenderAuthority);
    }
    InventorySource source { InventorySourceType::ContainerInventory, ContainerID::Inventory };
    auto&           actions = mTransaction->getActions(source);
    if (actions.size() != 1) { return origin(pPlayer, pIsSenderAuthority); }
    auto& item        = pPlayer.mInventory->mInventory->getItem(actions[0].mSlot);
    auto  beforeEvent = PlayerDropItemBeforeEvent(pPlayer, const_cast<ItemStack&>(item));
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return InventoryTransactionError::AuthorityMismatch; }
    auto result = origin(pPlayer, pIsSenderAuthority);
    if (result == InventoryTransactionError::NoError)
    {
        LLEventBus.publish(PlayerDropItemAfterEvent(pPlayer, item));
    }
    return result;
}

Event_Hook_Factory(PlayerDropItem, <PlayerDropItemEventHook1, PlayerDropItemEventHook2>);

} // namespace ila::mc::inline actor::inline player