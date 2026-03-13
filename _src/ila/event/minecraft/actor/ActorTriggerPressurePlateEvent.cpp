#include "ila/event/actor/ActorTriggerPressurePlateEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/entity/ActorEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/block/BasePressurePlateBlock.h>

namespace ila::mc::inline actor
{

void ActorTriggerPressurePlateBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"] = ListTag { mPos.x, mPos.y, mPos.z };
}

void ActorTriggerPressurePlateAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["pos"] = ListTag { mPos.x, mPos.y, mPos.z };
}

LL_TYPE_INSTANCE_HOOK(
    ActorTriggerPressurePlateEventHook,
    HookPriority::Normal,
    BasePressurePlateBlock,
    &BasePressurePlateBlock::$shouldTriggerEntityInside,
    bool,
    BlockSource&    pRegion,
    BlockPos const& pPos,
    Actor&          pActor
)
{
    auto before = ActorTriggerPressurePlateBeforeEvent(pActor, pPos);
    LLEventBus.publish(before);
    if (before.isCancelled()) { return false; }
    auto result = origin(pRegion, pPos, pActor);
    if (result) { LLEventBus.publish(ActorTriggerPressurePlateAfterEvent(pActor, pPos)); }
    return result;
}

Event_Hook_Factory(ActorTriggerPressurePlate, <ActorTriggerPressurePlateEventHook>);

} // namespace ila::mc::inline actor