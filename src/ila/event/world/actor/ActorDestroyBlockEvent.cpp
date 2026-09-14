#include "ila/event/world/actor/ActorDestroyBlockEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/ecs/gamerefs_entity/GameRefsEntity.h>
#include <mc/gameplayhandlers/ActorGameplayHandler.h>
#include <mc/world/events/ActorEventListener.h>
#include <mc/world/events/ActorGameplayEvent.h>
#include <mc/world/events/ActorGriefingBlockEvent.h>
#include <mc/world/events/EventCoordinatorPimpl.h>

namespace ila::mc::inline world::inline actor {

void ActorDestroyBlockEvent::serialize(CompoundTag& nbt) const {
    Cancellable::serialize(nbt);
    nbt["pos"] = ListTag{pos().x, pos().y, pos().z};
}
Vec3 const& ActorDestroyBlockEvent::pos() const { return mPos; }

LL_TYPE_INSTANCE_HOOK(
    ActorDestroyBlockEventHook,
    HookPriority::Normal,
    EventCoordinatorPimpl<ActorEventListener>,
    &EventCoordinatorPimpl<ActorEventListener>::_processEvent,
    CoordinatorResult,
    ActorGameplayHandler*                        handler,
    ActorGameplayEvent<CoordinatorResult> const& event
)
try {
    return event.visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Details::ValueOrRef<ActorGriefingBlockEvent const>>) {
            ActorGriefingBlockEvent const& griefingEvent = arg.value();
            auto beforeEvent = ActorDestroyBlockEvent(griefingEvent.mActorContext->tryUnwrap(), griefingEvent.mPos);
            LLEventBus.publish(beforeEvent);
            if (beforeEvent.isCancelled()) {
                origin(handler, event);
                return CoordinatorResult::Cancel;
            }
        }
        return origin(handler, event);
    });
} catch (...) {
    return origin(handler, event);
}

Event_Hook_Factory_Base(ActorDestroyBlock, <ActorDestroyBlockEventHook>);

} // namespace ila::mc::inline world::inline actor
