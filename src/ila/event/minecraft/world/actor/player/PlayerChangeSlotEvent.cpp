#include "ila/event/minecraft/world/actor/player/PlayerChangeSlotEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/EventRefObjSerializer.h>

namespace ila::mc::inline world::inline actor::inline player {
void PlayerChangeSlotEvent::serialize(CompoundTag& pNbt) const {
    Cancellable::serialize(pNbt);
    pNbt["container"]     = serializeRefObj(container());
    pNbt["slot"]          = slot();
    pNbt["oldItem"]       = serializeRefObj(oldItem());
    pNbt["newItem"]       = serializeRefObj(newItem());
    pNbt["forceBalanced"] = forceBalanced();
}

Container&       PlayerChangeSlotEvent::container() const { return mContainer; }
int&             PlayerChangeSlotEvent::slot() const { return mSlot; }
ItemStack const& PlayerChangeSlotEvent::oldItem() const { return mOldItem; }
ItemStack const& PlayerChangeSlotEvent::newItem() const { return mNewItem; }
bool&            PlayerChangeSlotEvent::forceBalanced() const { return mForceBalanced; }

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
) {
    auto pcse = PlayerChangeSlotEvent(*this, pContainer, pSlot, pOldItem, pNewItem, pForceBalanced);
    LLEventBus.publish(pcse);
    if (pcse.isCancelled()) {
        return;
    }
    origin(pContainer, pSlot, pOldItem, pNewItem, pForceBalanced);
}
} // namespace ila::mc::inline world::inline actor::inline player