#include "ila/base/Gloabl.h"
#include "ila/include_all.h"
#include "ll/api/memory/Hook.h"
#include "mc/nbt/CompoundTag.h"
#include <mc/world/level/Spawner.h>
#include <ll/api/utils/StacktraceUtils.h>

LL_AUTO_TYPE_INSTANCE_HOOK(
    TestHook,
    HookPriority::Normal,
    Spawner,
    &Spawner::$spawnItem,
    ItemActor*,
    BlockSource&     region,
    ItemStack const& inst,
    Actor*           spawner,
    Vec3 const&      pos,
    int              throwTime
)
{
    if (inst.getTypeName() == "minecraft:wheat")
    {
        SelfLogger.info(
            "TestHook: Spawned wheat\n{0}",
            ll::stacktrace_utils::toString(ll::Stacktrace::current())
        ); //
    }
    return origin(region, inst, spawner, pos, throwTime);
}