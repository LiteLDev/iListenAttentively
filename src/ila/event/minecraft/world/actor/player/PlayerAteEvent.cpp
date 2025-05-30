#include "ila/event/minecraft/world/actor/player/PlayerAteEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/actor/player/Inventory.h>
#include <mc/world/actor/player/PlayerInventory.h>
#include <mc/world/item/BucketItem.h>
#include <mc/world/item/MedicineItem.h>
#include <mc/world/item/PotionItem.h>


namespace ila::mc::inline world::inline actor::inline player
{

void PlayerAteBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable<ServerPlayerEvent>::serialize(nbt);
    nbt["item"] = serializeRefObj(mItem);
}

void PlayerAteBeforeEvent::deserialize(CompoundTag const& nbt) { PlayerEvent::deserialize(nbt); }

ItemStack& PlayerAteBeforeEvent::item() const { return mItem; }

void PlayerAteAfterEvent::serialize(CompoundTag& nbt) const
{
    ServerPlayerEvent::serialize(nbt);
    nbt["slot"] = mSlot;
}

void PlayerAteAfterEvent::deserialize(CompoundTag const& nbt) { PlayerEvent::deserialize(nbt); }

int PlayerAteAfterEvent::slot() const { return mSlot; }

LL_TYPE_INSTANCE_HOOK(
    PlayerCompleteUsingItemHook,
    HookPriority::Normal,
    Player,
    &Player::completeUsingItem,
    void
)
{
    const std::set<std::string> item_names { "minecraft:potion",
                                             "minecraft:milk_bucket",
                                             "minecraft:medicine" };
    auto                        slot = mItemInUse->mSlot->mUnk972fd9.as<int>();
    auto                        checked =
        mItemInUse->mItem->getItem()->isFood() || item_names.contains(mItemInUse->mItem->getTypeName());
    if (checked)
    {
        PlayerAteBeforeEvent before { *reinterpret_cast<ServerPlayer*>(this), mItemInUse->mItem };
        ll::event::EventBus::getInstance().publish(before);
        if (before.isCancelled()) return stopUsingItem();
    }
    origin();
    if (checked)
    {
        PlayerAteAfterEvent after { *reinterpret_cast<ServerPlayer*>(this), slot };
        ll::event::EventBus::getInstance().publish(after);
    }
}

Event_Hook_Factory(PlayerAte, <PlayerCompleteUsingItemHook>)

} // namespace ila::mc::inline world::inline actor::inline player