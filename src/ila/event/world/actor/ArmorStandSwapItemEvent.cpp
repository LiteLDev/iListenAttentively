#include "ila/event/world/actor/ArmorStandSwapItemEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/entity/components_json_legacy/Interaction.h>
#include <mc/world/actor/ArmorStand.h>
#include <mc/world/actor/provider/ActorEquipment.h>
#include <mc/world/item/ItemStack.h>

namespace ila::mc::inline world::inline actor {

void ArmorStandSwapItemBeforeEvent::serialize(CompoundTag& nbt) const {
    Cancellable::serialize(nbt);
    nbt["player"] = serializeRefObj(player());
    nbt["slot"]   = magic_enum::enum_name(slot());
}
void ArmorStandSwapItemBeforeEvent::deserialize(CompoundTag const& nbt) {
    Cancellable::deserialize(nbt);
    slot() = magic_enum::enum_cast<SharedTypes::Legacy::EquipmentSlot>(nbt["slot"].get<StringTag>()).value_or(slot());
}
Player&                             ArmorStandSwapItemBeforeEvent::player() const { return mPlayer; }
SharedTypes::Legacy::EquipmentSlot& ArmorStandSwapItemBeforeEvent::slot() const { return mSlot; }

void ArmorStandSwapItemAfterEvent::serialize(CompoundTag& nbt) const {
    ActorEvent::serialize(nbt);
    nbt["player"] = serializeRefObj(player());
    nbt["slot"]   = magic_enum::enum_name(slot());
}
Player const&                             ArmorStandSwapItemAfterEvent::player() const { return mPlayer; }
SharedTypes::Legacy::EquipmentSlot const& ArmorStandSwapItemAfterEvent::slot() const { return mSlot; }

LL_TYPE_INSTANCE_HOOK(
    ArmorStandSwapItemEventHook,
    HookPriority::Normal,
    ArmorStand,
    &ArmorStand::$getInteraction,
    Interaction,
    Player&      pPlayer,
    Vec3 const&  pLocation,
    HandSlot     pHandSlot
) {
    auto const& held = pHandSlot == HandSlot::Offhand
                         ? ActorEquipment::getHandContainer(pPlayer.mEntityContext).getItem(1)
                         : pPlayer.getCarriedItem();
    SharedTypes::Legacy::EquipmentSlot slot = SharedTypes::Legacy::EquipmentSlot::None;
    if (!held.isNull() && held.isHumanoidWearableItem()) {
        slot = static_cast<SharedTypes::Legacy::EquipmentSlot>(static_cast<int>(held.getArmorSlot()) + 2);
    }
    if (slot == SharedTypes::Legacy::EquipmentSlot::None) {
        return origin(pPlayer, pLocation, pHandSlot);
    }

    auto beforeEvent = ArmorStandSwapItemBeforeEvent(*this, pPlayer, slot);
    LLEventBus.publish(beforeEvent);
    auto interaction = origin(pPlayer, pLocation, pHandSlot);
    if (beforeEvent.isCancelled()) {
        interaction.mCallback.get() = nullptr;
        return interaction;
    }
    if (interaction.mCallback.get()) {
        auto original = std::move(interaction.mCallback.get());
        interaction.mCallback.get() = [this, &pPlayer, slot, original = std::move(original)]() mutable {
            std::move(original)();
            LLEventBus.publish(ArmorStandSwapItemAfterEvent(*this, pPlayer, slot));
        };
    }
    return interaction;
}

Event_Hook_Factory(ArmorStandSwapItem, <ArmorStandSwapItemEventHook>);
} // namespace ila::mc::inline world::inline actor
