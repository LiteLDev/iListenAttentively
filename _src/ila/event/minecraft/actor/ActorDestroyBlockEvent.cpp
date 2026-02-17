#pragma include_alias("mc/world/events/ActorGriefingBlockEvent.h", "ila/patch/ActorGriefingBlockEvent.hpp")
#include "ila/event/minecraft/actor/ActorDestroyBlockEvent.h"
#include "ila/base/Gloabl.h"
#include <ila/patch/ActorGriefingBlockEvent.hpp>
#include <ll/api/event/Cancellable.h>
#include <ll/api/memory/Hook.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/ecs/gamerefs_entity/GameRefsEntity.h>
#include <mc/gameplayhandlers/CoordinatorResult.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/world/events/ActorEventCoordinator.h>
#include <mc/world/events/ActorGameplayEvent.h>
#include <mc/world/events/EventRef.h>
#include <mc/world/events/details/ValueOrRef.h>
#include <type_traits>

namespace ila::mc::inline actor
{

void ActorDestroyBlockEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"] = ListTag { mPos.x, mPos.y, mPos.z };
}

LL_TYPE_INSTANCE_HOOK(
    ActorDestroyBlockEventHook,
    HookPriority::Normal,
    ActorEventCoordinator,
    &ActorEventCoordinator::sendEvent,
    CoordinatorResult,
    EventRef<ActorGameplayEvent<CoordinatorResult>> const& event
)
try
{
    return event.get().visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Details::ValueOrRef<ActorGriefingBlockEvent const>>)
        {
            ActorGriefingBlockEvent const& griefingEvent = arg.value();
            auto                           beforeEvent =
                ActorDestroyBlockEvent(griefingEvent.mActorContext.tryUnwrap(), griefingEvent.mPos);
            LLEventBus.publish(beforeEvent);
            if (beforeEvent.isCancelled())
            {
                origin(event);
                return CoordinatorResult::Cancel;
            }
        }
        return origin(event);
    });
}
catch (...)
{
    return origin(event);
}

Event_Hook_Factory_Base(ActorDestroyBlock, <ActorDestroyBlockEventHook>);

} // namespace ila::mc::inline actor