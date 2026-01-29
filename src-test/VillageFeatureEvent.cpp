#include "ila/event/minecraft/worldgen/structure/VillageFeatureEvent.h"
#include "ll/api/base/FixedString.h"
#include "ll/api/event/Cancellable.h"
#include "ll/api/event/DynamicListener.h"
#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/MultiListener.h"
#include "ll/api/memory/Hook.h"
#include "mc/server/ServerInstance.h"
#include "mc/world/events/ServerInstanceEventCoordinator.h"


LL_AUTO_TYPE_INSTANCE_HOOK(
    EventTestHook,
    ll::memory::HookPriority::Normal,
    ServerInstanceEventCoordinator,
    &ServerInstanceEventCoordinator::sendServerInitializeEnd,
    void,
    ::ServerInstance& ins
)
{
    ll::event::EventBus::getInstance().emplaceListener<ila::mc::worldgen::structure::VillageFeatureConstructionEvent>(
        [this]([[maybe_unused]] ila::mc::worldgen::structure::VillageFeatureConstructionEvent& ev) {}
    );
    origin(ins);
}
