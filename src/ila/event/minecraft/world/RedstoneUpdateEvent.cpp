#include "ila/event/minecraft/world/RedstoneUpdateEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/memory/Symbol.h>
#include <mc/deps/nbt/CompoundTag.h>
#include <mc/deps/nbt/ListTag.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/ObserverBlock.h>
#include <mc/world/level/block/block_events/BlockEventExecutor.h>
#include <mc/world/level/block/block_events/BlockEventManager.h>
#include <mc/world/level/block/block_events/BlockRedstoneUpdateEvent.h>
#include <mc/world/redstone/circuit/ChunkCircuitComponentList.h>
#include <mc/world/redstone/circuit/CircuitSceneGraph.h>
#include <mc/world/redstone/circuit/CircuitSystem.h>
#include <mc/world/redstone/circuit/components/BaseCircuitComponent.h>
#include <vector>

namespace ila::mc::inline world
{

void RedstoneUpdateBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]         = ListTag { pos().x, pos().y, pos().z };
    nbt["strength"]    = strength();
    nbt["isFirstTime"] = isFirstTime();
    nbt["dimId"]       = getDimensionName(blockSource());
}
void RedstoneUpdateBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x       = nbt["pos"][0];
    pos().y       = nbt["pos"][1];
    pos().z       = nbt["pos"][2];
    strength()    = nbt["strength"];
    isFirstTime() = nbt["isFirstTime"];
}
BlockPos& RedstoneUpdateBeforeEvent::pos() const { return mPos; }
int&      RedstoneUpdateBeforeEvent::strength() const { return mStrength; }
bool&     RedstoneUpdateBeforeEvent::isFirstTime() const { return mIsFirstTime; }

void RedstoneUpdateAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]         = ListTag { pos().x, pos().y, pos().z };
    nbt["strength"]    = strength();
    nbt["isFirstTime"] = isFirstTime();
    nbt["dimId"]       = getDimensionName(blockSource());
}
BlockPos const& RedstoneUpdateAfterEvent::pos() const { return mPos; }
int const&      RedstoneUpdateAfterEvent::strength() const { return mStrength; }
bool const&     RedstoneUpdateAfterEvent::isFirstTime() const { return mIsFirstTime; }

// ObserverBlock::_startSignal no longer exists since 1.26.40 (inlined into neighborChanged), the observer
// emits its 15 strength pulse in _updateState instead, so hook it with turnOn as the pulse gate.
LL_TYPE_INSTANCE_HOOK(
    RedstoneUpdateEventHook1,
    HookPriority::Normal,
    ObserverBlock,
    &ObserverBlock::_updateState,
    void,
    BlockSource&    pRegion,
    BlockPos const& pPos,
    PulseCapacitor& pComponent,
    bool            pTurnOn
)
{
    if (!pTurnOn) { return origin(pRegion, pPos, pComponent, pTurnOn); }

    int  strength    = 15;
    bool isFirstTime = false;
    auto beforeEvent = RedstoneUpdateBeforeEvent(pRegion, const_cast<BlockPos&>(pPos), strength, isFirstTime);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pRegion, pPos, pComponent, pTurnOn);
    LLEventBus.publish(RedstoneUpdateAfterEvent(pRegion, pPos, strength, isFirstTime));
}

// Restored from Block::onRedstoneUpdate (client-only in the LeviLamina headers, inlined since 1.26.40).
// It only forwards a BlockEvents::BlockRedstoneUpdateEvent to the block type's event executor.
void Block_onRedstoneUpdate(
    ::Block const&    block,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    short             strength,
    short             oldStrength,
    bool              isFirstTime
)
{
    auto* executor = static_cast<BlockEvents::BlockEventExecutor<BlockEvents::BlockRedstoneUpdateEvent>*>(
        block.mBlockType->mEventManager->_tryGetExecutor(BlockEvents::EventType::RedstoneUpdate)
    );
    if (executor == nullptr) { return; }

    auto event = BlockEvents::BlockRedstoneUpdateEvent(pos, region, strength, oldStrength, isFirstTime);

    executor->dispatch(event);
}

// Restored from CircuitSystem::updateIndividualBlock (private, undeclared in the headers).
// The original ignores its 3rd parameter (pos) and uses the 4th one (region) as the block pos.
void CircuitSystem_updateIndividualBlock(
    CircuitSystem*                           sys,
    ::gsl::not_null<::BaseCircuitComponent*> component,
    ::BlockPos const&                        pos,
    ::BlockPos const&                        region,
    ::BlockSource&                           blockSource
)
{
    int   strength    = component->getStrength();
    short oldStrength = component->mOldStrength;
    component->setOldStrength(static_cast<short>(strength));
    if (static_cast<short>(strength) != -1)
    {
        ::Block const& block       = blockSource.getBlock(region);
        bool           isFirstTime = component->mIsFirstTime;
        if (!isFirstTime || !component->mIgnoreFirstUpdate)
        {
            Block_onRedstoneUpdate(
                block,
                blockSource,
                region,
                static_cast<short>(strength),
                oldStrength,
                isFirstTime
            );
        }
        component->mIsFirstTime = false;
    }
}

LL_TYPE_INSTANCE_HOOK(
    RedstoneUpdateEventHook2,
    HookPriority::Low,
    CircuitSystem,
    &CircuitSystem::updateBlocks,
    void,
    BlockSource&    pRegion,
    BlockPos const& pChunkPos
)
{
    if (!mHasBeenEvaluated) { return; }
    auto& activeComponents = mSceneGraph->mActiveComponentsPerChunk;
    if (activeComponents.empty()) { return; }

    auto const& components = activeComponents.find(pChunkPos);
    if (components == activeComponents.end()) { return; }

    std::vector<ChunkCircuitComponentList::Item> secondaryPoweredList;
    secondaryPoweredList.reserve(components->second.mComponents->size());

    auto processComponent = [&](BaseCircuitComponent* comp, BlockPos const& pos) -> void {
        int strength = comp->getStrength();
        if (strength == -1) { return; }

        bool doEvent = false;
        if (!comp->mIsFirstTime || !comp->mIgnoreFirstUpdate)
        {
            auto beforeEvent =
                RedstoneUpdateBeforeEvent(pRegion, const_cast<BlockPos&>(pos), strength, comp->mIsFirstTime);
            LLEventBus.publish(beforeEvent);
            if (beforeEvent.isCancelled()) { return; }
            doEvent = true;
        }

        bool& usedIsFirstTime = comp->mIsFirstTime;
        CircuitSystem_updateIndividualBlock(this, comp, pChunkPos, pos, pRegion);

        if (doEvent)
        {
            LLEventBus.publish(RedstoneUpdateAfterEvent(pRegion, pos, strength, usedIsFirstTime));
        }
    };

    for (auto& item : *components->second.mComponents)
    {
        if (auto* comp = item.mComponent; comp && !comp->mRemoved && comp->mNeedsUpdate)
        {
            comp->mNeedsUpdate = false;
            if (comp->isSecondaryPowered()) { secondaryPoweredList.emplace_back(item); }
            else
            {
                processComponent(comp, item.mPos);
            }
        }
    }

    for (auto const& item : secondaryPoweredList)
    {
        if (auto* comp = item.mComponent; comp) { processComponent(comp, item.mPos); }
    }
}

Event_Hook_Factory(RedstoneUpdate, <RedstoneUpdateEventHook1, RedstoneUpdateEventHook2>);

} // namespace ila::mc::inline world
