#include "ila/event/minecraft/block/SculkCatalystAbsorbExperienceEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/world/LevelEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/level/IBlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/actor/SculkCatalystBlockActor.h>

namespace ila::mc::inline block
{

void SculkCatalystAbsorbExperienceBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["blockActor"] = serializeRefObj(blockActor());
    nbt["actor"]      = serializeRefObj(actor());
}
SculkCatalystBlockActor& SculkCatalystAbsorbExperienceBeforeEvent::blockActor() const { return mBlockActor; };
Actor&                   SculkCatalystAbsorbExperienceBeforeEvent::actor() const { return mActor; };

void SculkCatalystAbsorbExperienceAfterEvent::serialize(CompoundTag& nbt) const
{
    LevelEvent::serialize(nbt);
    nbt["blockActor"] = serializeRefObj(blockActor());
    nbt["actor"]      = serializeRefObj(actor());
}
SculkCatalystBlockActor& SculkCatalystAbsorbExperienceAfterEvent::blockActor() const { return mBlockActor; };
Actor&                   SculkCatalystAbsorbExperienceAfterEvent::actor() const { return mActor; };

LL_TYPE_INSTANCE_HOOK(
    SculkCatalystAbsorbExperienceEventHook,
    HookPriority::Normal,
    SculkCatalystBlockActor,
    &SculkCatalystBlockActor::_tryConsumeOnDeathExperience,
    void,
    Level& pLevel,
    Actor& pActor
)
{
    auto beforeEvent = SculkCatalystAbsorbExperienceBeforeEvent(pLevel, *this, pActor);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pLevel, pActor);
    LLEventBus.publish(SculkCatalystAbsorbExperienceAfterEvent(pLevel, *this, pActor));
}

Event_Hook_Factory(SculkCatalystAbsorbExperience, <SculkCatalystAbsorbExperienceEventHook>);

} // namespace ila::mc::inline block