#include "ila/event/minecraft/world/actor/DragonRespawnEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/service/Bedrock.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/dimension/end/EndDragonFight.h>

namespace ila::mc::inline world::inline actor
{

LL_TYPE_INSTANCE_HOOK(
    DragonRespawnEventHook,
    HookPriority::Normal,
    EndDragonFight,
    &EndDragonFight::_createNewDragon,
    void,
)
{
    if (mPreviouslyKilled)
    {
        auto beforeEvent = DragonRespawnBeforeEvent();
        LLEventBus.publish(beforeEvent);
        if (beforeEvent.isCancelled()) { return; }
        origin();
        if (auto* dragon = ll::service::getLevel()->fetchEntity(mDragonUUID, false))
        {
            LLEventBus.publish(DragonRespawnAfterEvent(static_cast<EnderDragon&>(*dragon)));
        }
    }
    else
    {
        origin();
    }
}

Event_Hook_Factory(DragonRespawn, <DragonRespawnEventHook>);

} // namespace ila::mc::inline world::inline actor
