#include "ila/event/minecraft/world/RedstoneUpdateEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/BlockType.h>
#include <mc/world/level/block/ObserverBlock.h>
#include <mc/world/redstone/circuit/ChunkCircuitComponentList.h>
#include <mc/world/redstone/circuit/CircuitSceneGraph.h>
#include <mc/world/redstone/circuit/CircuitSystem.h>
#include <mc/world/redstone/circuit/components/BaseCircuitComponent.h>

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

LL_TYPE_INSTANCE_HOOK(
    RedstoneUpdateEventHook1,
    HookPriority::Normal,
    ObserverBlock,
    &ObserverBlock::_startSignal,
    void,
    BlockSource&    pRegion,
    BlockPos const& pPos
)
{
    int  strength    = 15;
    bool isFirstTime = false;
    auto beforeEvent = RedstoneUpdateBeforeEvent(pRegion, const_cast<BlockPos&>(pPos), strength, isFirstTime);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pRegion, pPos);
    LLEventBus.publish(RedstoneUpdateAfterEvent(pRegion, pPos, strength, isFirstTime));
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

    constexpr static auto processComponent =
        [](BaseCircuitComponent* comp, BlockSource& region, BlockPos const& pos) -> void {
        if (auto strength = comp->getStrength(); strength != -1)
        {
            auto& block = region.getBlock(pos);
            if (!comp->mIsFirstTime || !comp->mIgnoreFirstUpdate)
            {
                auto beforeEvent = RedstoneUpdateBeforeEvent(
                    region,
                    const_cast<BlockPos&>(pos),
                    strength,
                    comp->mIsFirstTime
                );
                LLEventBus.publish(beforeEvent);
                if (beforeEvent.isCancelled()) { return; }
                block.mBlockType->onRedstoneUpdate(region, pos, strength, comp->mIsFirstTime);
                LLEventBus.publish(RedstoneUpdateAfterEvent(region, pos, strength, comp->mIsFirstTime));
            }
            comp->mIsFirstTime = false;
        }
    };

    for (auto& item : *components->second.mComponents)
    {
        if (auto* comp = item.mComponent; comp && !comp->mRemoved && comp->mNeedsUpdate)
        {
            comp->mNeedsUpdate = false;
            if (comp->isSecondaryPowered()) { secondaryPoweredList.emplace_back(item); }
            else { processComponent(comp, pRegion, item.mPos); }
        }
    }

    for (auto const& item : secondaryPoweredList)
    {
        if (auto* comp = item.mComponent; comp) { processComponent(comp, pRegion, item.mPos); }
    }
}

Event_Hook_Factory(RedstoneUpdate, <RedstoneUpdateEventHook1, RedstoneUpdateEventHook2>);

} // namespace ila::mc::inline world