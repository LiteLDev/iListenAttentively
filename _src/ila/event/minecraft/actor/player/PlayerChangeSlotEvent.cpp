#include "ila/event/actor/player/PlayerChangeSlotEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/memory/Hook.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/Container.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/item/ItemStack.h>

namespace ila::mc::inline actor::inline player
{
void PlayerChangeSlotEvent::serialize(CompoundTag& pNbt) const
{
    Cancellable::serialize(pNbt);
    pNbt["container"]     = serializeRefObj(mContainer);
    pNbt["slot"]          = mSlot;
    pNbt["oldItem"]       = serializeRefObj(mOldItem);
    pNbt["newItem"]       = serializeRefObj(mNewItem);
    pNbt["forceBalanced"] = mForceBalanced;
}

LL_TYPE_INSTANCE_HOOK(
    PlayerChangeSlotHook,
    HookPriority::Normal,
    Player,
    &Player::inventoryChanged,
    void,
    Container&       pContainer,
    int              pSlot,
    ItemStack const& pOldItem,
    ItemStack const& pNewItem,
    bool             pForceBalanced
)
{
    auto pcse = PlayerChangeSlotEvent(*this, pContainer, pSlot, pOldItem, pNewItem, pForceBalanced);
    LLEventBus.publish(pcse);
    if (pcse.isCancelled()) { return; }
    origin(pContainer, pSlot, pOldItem, pNewItem, pForceBalanced);
}
} // namespace ila::mc::inline actor::inline player