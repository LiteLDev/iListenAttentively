#include "ila/event/world/level/block/SculkCatalystAbsorbExperienceEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/events/gameevents/GameEventContext.h>

namespace ila::mc::inline world::inline level::inline block {

void SculkCatalystAbsorbExperienceBeforeEvent::serialize(CompoundTag& nbt) const {
    Cancellable::serialize(nbt);
    nbt["blockActor"] = serializeRefObj(blockActor());
    nbt["actor"]      = serializeRefObj(actor());
}
SculkCatalystBlockActor& SculkCatalystAbsorbExperienceBeforeEvent::blockActor() const { return mBlockActor; };
Actor&                   SculkCatalystAbsorbExperienceBeforeEvent::actor() const { return mActor; };

void SculkCatalystAbsorbExperienceAfterEvent::serialize(CompoundTag& nbt) const {
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
    &SculkCatalystBlockActor::$handleGameEvent,
    void,
    GameEvent const&        gameEvent,
    GameEventContext const& gameEventContext,
    BlockSource&            region
) {
    if (Actor* const actor = gameEventContext.mSource) {
        auto beforeEvent = SculkCatalystAbsorbExperienceBeforeEvent(region.getLevel(), *this, *actor);
        LLEventBus.publish(beforeEvent);
        if (beforeEvent.isCancelled()) {
            return;
        }
        origin(gameEvent, gameEventContext, region);
        LLEventBus.publish(SculkCatalystAbsorbExperienceAfterEvent(region.getLevel(), *this, *actor));
    } else {
        origin(gameEvent, gameEventContext, region);
    }
}

Event_Hook_Factory(SculkCatalystAbsorbExperience, <SculkCatalystAbsorbExperienceEventHook>);

} // namespace ila::mc::inline world::inline level::inline block
