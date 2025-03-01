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
    &EndDragonFight::_setRespawnStage,
    void,
    RespawnAnimation pStage
)
{
    auto beforeEvent = DragonRespawnBeforeEvent();
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pStage);
    if (auto* dragon = ll::service::getLevel()->fetchEntity(mDragonUUID, false))
    {
        LLEventBus.publish(DragonRespawnAfterEvent(static_cast<EnderDragon&>(*dragon)));
    }
}

Event_Hook_Factory(DragonRespawn, <DragonRespawnEventHook>);

} // namespace ila::mc::inline world::inline actor