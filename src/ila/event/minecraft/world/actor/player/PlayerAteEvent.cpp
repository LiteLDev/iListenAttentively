#include "ila/event/minecraft/world/actor/player/PlayerAteEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/actor/player/Inventory.h>
#include <mc/world/actor/player/PlayerInventory.h>
#include <mc/world/item/BucketItem.h>
#include <mc/world/item/MedicineItem.h>
#include <mc/world/item/PotionItem.h>
#include <mc/world/item/VanillaItemNames.h>

namespace ila::mc::inline world::inline actor::inline player
{

void PlayerAteBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["item"] = serializeRefObj(item());
}
ItemStack& PlayerAteBeforeEvent::item() const { return mItem; }

void PlayerAteAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["slot"] = slot();
}
int PlayerAteAfterEvent::slot() const { return mSlot; }

LL_TYPE_INSTANCE_HOOK(
    PlayerCompleteUsingItemHook,
    HookPriority::Normal,
    Player,
    &Player::completeUsingItem,
    void
)
{
    static std::set<std::string> mItmemNames = { VanillaItemNames::Potion(),
                                                 VanillaItemNames::MilkBucket(),
                                                 VanillaItemNames::Medicine() };
    auto                         slot        = mItemInUse->mSlot->mSlot;
    if (!mItemInUse->mItem->getItem()->isFood() && !mItmemNames.contains(mItemInUse->mItem->getTypeName()))
    {
        return origin();
    }
    PlayerAteBeforeEvent before(*this, mItemInUse->mItem);
    LLEventBus.publish(before);
    if (before.isCancelled()) return stopUsingItem();
    origin();
    LLEventBus.publish(PlayerAteAfterEvent(*this, slot));
}

Event_Hook_Factory(PlayerAte, <PlayerCompleteUsingItemHook>)

} // namespace ila::mc::inline world::inline actor::inline player