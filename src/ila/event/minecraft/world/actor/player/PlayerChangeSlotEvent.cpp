#include "ila/event/minecraft/world/actor/player/PlayerChangeSlotEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/EventRefObjSerializer.h>

namespace ila::mc::inline world::inline actor::inline player
{
void PlayerChangeSlotEvent::serialize(CompoundTag& pNbt) const
{
    Cancellable::serialize(pNbt);
    pNbt["container"]     = ll::event::serializeRefObj(this->container());
    pNbt["slot"]          = this->slot();
    pNbt["oldItem"]       = ll::event::serializeRefObj(this->oldItem());
    pNbt["newItem"]       = ll::event::serializeRefObj(this->newItem());
    pNbt["forceBalanced"] = this->forceBalanced();
}

Container&       PlayerChangeSlotEvent::container() const { return this->mContainer; }
int&             PlayerChangeSlotEvent::slot() const { return this->mSlot; }
ItemStack const& PlayerChangeSlotEvent::oldItem() const { return this->mOldItem; }
ItemStack const& PlayerChangeSlotEvent::newItem() const { return this->mNewItem; }
bool&            PlayerChangeSlotEvent::forceBalanced() const { return this->mForceBalanced; }

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
} // namespace ila::mc::inline world::inline actor::inline player