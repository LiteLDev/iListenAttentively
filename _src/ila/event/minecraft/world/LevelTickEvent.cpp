#include "ila/event/world/LevelTickEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/memory/Hook.h>
#include <mc/world/level/Level.h>

namespace ila::mc::inline world
{

LL_TYPE_INSTANCE_HOOK(LevelTickEventHook, HookPriority::Normal, Level, &Level::$tick, void)
{
    auto beforeEvent = LevelTickBeforeEvent(*this);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin();
    LLEventBus.publish(LevelTickAfterEvent(*this));
}

Event_Hook_Factory(LevelTick, <LevelTickEventHook>);

} // namespace ila::mc::inline world