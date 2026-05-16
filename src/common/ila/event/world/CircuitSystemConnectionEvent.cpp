#include "ila/event/world/CircuitSystemConnectionEvent.h"
#include "ila/utils/EventUtils.i.h"
#include <ila/base/Gloabl.i.h>
#include <mc/world/redstone/circuit/CircuitSceneGraph.h>
#include <mc/world/redstone/circuit/components/BaseCircuitComponent.h>
#include <mc/world/redstone/circuit/CircuitTrackingInfo.h>

namespace ila::world {

void CircuitSystemConnectionEvent::serialize(CompoundTag& nbt) const {
    WorldEvent::serialize(nbt);
    reflection::serialize_to(nbt["from_pos"], mFromPos).value();
    reflection::serialize_to(nbt["to_pos"], mToPos).value();
}

static optional_ref<BlockSource> gRegion;

LL_TYPE_INSTANCE_HOOK(
    CircuitSystemConnectionEventHook1,
    HookPriority::Normal,
    CircuitSceneGraph,
    &CircuitSceneGraph::update,
    void,
    BlockSource* region
) {
    gRegion = region;
    origin(region);
    gRegion = nullptr;
}

LL_STATIC_HOOK(
    CircuitSystemConnectionEventHook2,
    HookPriority::Normal,
    // "addToFillQueue"_sym,
    "4D 85 C0 0F 84 ?? ?? ?? ?? 4C 8B DC 55"_sig,
    void,
    CircuitSceneGraph&               graph,
    CircuitComponentList&            powerAssociationMap,
    BaseCircuitComponent*            newComponent,
    CircuitTrackingInfo&             info,
    BlockPos const&                  newPos,
    uchar                            face,
    std::queue<CircuitTrackingInfo>& positions
) {
    if (!newComponent) return;

    CircuitTrackingInfo newInfo{};
    newInfo.mCurrent             = info.mCurrent;
    newInfo.mPower               = info.mPower;
    newInfo.mNearest             = info.mNearest;
    newInfo.m2ndNearest          = info.m2ndNearest;
    newInfo.mDampening           = info.mDampening;
    newInfo.mDirectlyPowered     = info.mDirectlyPowered;
    newInfo.mCurrent->mComponent = newComponent;
    newInfo.mCurrent->mPos       = newPos;
    newInfo.mCurrent->mDirection = face;
    newInfo.mCurrent->mTypeID    = newComponent->getCircuitComponentGroupType();

    CircuitComponentList::Item item{};
    item.mComponent = newComponent;
    item.mPos       = newPos;
    item.mData      = 6;

    powerAssociationMap.mComponents.emplace_back(item);

    if (info.mNearest->mComponent->allowConnection(graph, newInfo, newInfo.mDirectlyPowered)
        && (!gRegion || !eventPromise(CircuitSystemConnectingEvent{*gRegion, info.mCurrent->mPos, newPos}).publish())
        && newComponent->addSource(graph, newInfo, newInfo.mDampening, newInfo.mDirectlyPowered)) {
        newInfo.m2ndNearest          = info.mNearest;
        newInfo.m2ndNearest->mPos    = info.mNearest->mPos;
        newInfo.mNearest->mComponent = newComponent;
        newInfo.mNearest->mPos       = newPos;
        newInfo.mNearest->mDirection = face;
        newInfo.mNearest->mTypeID    = newInfo.mCurrent->mTypeID;
        positions.push(std::move(newInfo));
        eventPromise(CircuitSystemConnectedEvent{*gRegion, info.mCurrent->mPos, newPos}).publish();
    }
}

EventHook(
    CircuitSystemConnectingEvent,
    CircuitSystemConnectedEvent,
    <CircuitSystemConnectionEventHook1, CircuitSystemConnectionEventHook2>
);

} // namespace ila::world

CircuitTrackingInfo::CircuitTrackingInfo() = default;