#include "ila/event/actor/ActorTickEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/memory/Hook.h>
#include <mc/world/actor/Actor.h>

namespace ila::mc::inline actor
{
LL_TYPE_INSTANCE_HOOK(ActorTickEventHook, HookPriority::Normal, Actor, &Actor::$normalTick, void)
{
    auto beforeEvent = ActorTickBeforeEvent(*this);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin();
    LLEventBus.publish(ActorTickAfterEvent(*this));
}

Event_Hook_Factory(ActorTick, <ActorTickEventHook>);

} // namespace ila::mc::inline actor