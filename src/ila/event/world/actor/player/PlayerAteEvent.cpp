#include "ila/event/world/actor/player/PlayerAteEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/service/Bedrock.h>
#include <mc/server/ServerInstance.h>
#include <mc/world/actor/player/Inventory.h>
#include <mc/world/actor/player/PlayerInventory.h>
#include <mc/world/item/BucketItem.h>
#include <mc/world/item/MedicineItem.h>
#include <mc/world/item/PotionItem.h>
#include <mc/world/item/VanillaItemNames.h>

namespace ila::mc::inline world::inline actor::inline player {

void PlayerAteBeforeEvent::serialize(CompoundTag& nbt) const {
    Cancellable::serialize(nbt);
    nbt["item"] = serializeRefObj(item());
}
ItemStack& PlayerAteBeforeEvent::item() const { return mItem; }

void PlayerAteAfterEvent::serialize(CompoundTag& nbt) const {
    PlayerEvent::serialize(nbt);
    nbt["slot"] = slot();
}
int PlayerAteAfterEvent::slot() const { return mSlot; }

LL_TYPE_INSTANCE_HOOK(PlayerCompleteUsingItemHook, HookPriority::Normal, Player, &Player::completeUsingItem, void) {
    if (std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id()) {
        return origin();
    }
    static std::set<std::string> mItmemNames = {"minecraft:potion", "minecraft:milk_bucket", "minecraft:medicine"};
    auto                         slot        = mItemInUse->mSlot->mSlot;
    if (!mItemInUse->mItem->mItem->isFood() && !mItmemNames.contains(mItemInUse->mItem->getTypeName())) {
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
