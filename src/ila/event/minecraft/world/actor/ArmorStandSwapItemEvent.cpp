#include "ila/event/minecraft/world/actor/ArmorStandSwapItemEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/actor/ArmorStand.h>

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
    &ArmorStand::_trySwapItem,
    bool,
    Player&                            pPlayer,
    SharedTypes::Legacy::EquipmentSlot pSlot
) {
    auto beforeEvent = ArmorStandSwapItemBeforeEvent(*this, pPlayer, pSlot);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) {
        return false;
    }
    auto result = origin(pPlayer, pSlot);
    if (result) {
        LLEventBus.publish(ArmorStandSwapItemAfterEvent(*this, pPlayer, pSlot));
    }
    return result;
}

Event_Hook_Factory(ArmorStandSwapItem, <ArmorStandSwapItemEventHook>);
} // namespace ila::mc::inline world::inline actor