#include "ila/event/minecraft/actor/player/PlayerAteEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/player/PlayerEvent.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/service/Bedrock.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/server/ServerInstance.h>
#include <mc/world/actor/player/Inventory.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/actor/player/PlayerInventory.h>
#include <mc/world/item/BucketItem.h>
#include <mc/world/item/ItemStack.h>
#include <mc/world/item/MedicineItem.h>
#include <mc/world/item/PotionItem.h>
#include <mc/world/item/VanillaItemNames.h>
#include <set>
#include <string>
#include <thread>

namespace ila::mc::inline actor::inline player
{

void PlayerAteBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["item"] = serializeRefObj(mItem);
}

void PlayerAteAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["slot"] = mSlot;
}

LL_TYPE_INSTANCE_HOOK(
    PlayerCompleteUsingItemHook,
    HookPriority::Normal,
    Player,
    &Player::completeUsingItem,
    void
)
{
    if (std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
    {
        return origin();
    }
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

} // namespace ila::mc::inline actor::inline player