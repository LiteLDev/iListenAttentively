#include "ila/event/actor/ArmorStandSwapItemEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/entity/ActorEvent.h>
#include <ll/api/memory/Hook.h>
#include <magic_enum.hpp>
#include <mc/deps/shared_types/legacy/EquipmentSlot.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/StringTag.h>
#include <mc/world/actor/ArmorStand.h>
#include <mc/world/actor/player/Player.h>

namespace ila::mc::inline actor
{

void ArmorStandSwapItemBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["player"] = serializeRefObj(mPlayer);
    nbt["slot"]   = magic_enum::enum_name(mSlot);
}
void ArmorStandSwapItemBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mSlot = magic_enum::enum_cast<SharedTypes::Legacy::EquipmentSlot>(nbt["slot"].get<StringTag>())
                 .value_or(mSlot);
}

void ArmorStandSwapItemAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["player"] = serializeRefObj(mPlayer);
    nbt["slot"]   = magic_enum::enum_name(mSlot);
}

LL_TYPE_INSTANCE_HOOK(
    ArmorStandSwapItemEventHook,
    HookPriority::Normal,
    ArmorStand,
    &ArmorStand::_trySwapItem,
    bool,
    Player&                            pPlayer,
    SharedTypes::Legacy::EquipmentSlot pSlot
)
{
    auto beforeEvent = ArmorStandSwapItemBeforeEvent(*this, pPlayer, pSlot);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return false; }
    auto result = origin(pPlayer, pSlot);
    if (result) { LLEventBus.publish(ArmorStandSwapItemAfterEvent(*this, pPlayer, pSlot)); }
    return result;
}

Event_Hook_Factory(ArmorStandSwapItem, <ArmorStandSwapItemEventHook>);
} // namespace ila::mc::inline actor