#include "ila/event/minecraft/world/RedstoneUpdateEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/BlockLegacy.h>
#include <mc/world/redstone/circuit/ChunkCircuitComponentList.h>
#include <mc/world/redstone/circuit/CircuitSceneGraph.h>
#include <mc/world/redstone/circuit/CircuitSystem.h>
#include <mc/world/redstone/circuit/components/BaseCircuitComponent.h>

namespace ila::mc::inline world
{

void RedstoneUpdateBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]         = ListTag { getPos().x, getPos().y, getPos().z };
    nbt["strength"]    = getStrength();
    nbt["isFirstTime"] = getIsFirstTime();
}
void RedstoneUpdateBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    getPos().x       = nbt["pos"]["x"];
    getPos().y       = nbt["pos"]["y"];
    getPos().z       = nbt["pos"]["z"];
    getStrength()    = nbt["strength"];
    getIsFirstTime() = nbt["isFirstTime"];
}
BlockPos& RedstoneUpdateBeforeEvent::getPos() const { return mPos; }
int&      RedstoneUpdateBeforeEvent::getStrength() const { return mStrength; }
bool&     RedstoneUpdateBeforeEvent::getIsFirstTime() const { return mIsFirstTime; }

void RedstoneUpdateAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]         = ListTag { getPos().x, getPos().y, getPos().z };
    nbt["strength"]    = getStrength();
    nbt["isFirstTime"] = getIsFirstTime();
}
BlockPos const& RedstoneUpdateAfterEvent::getPos() const { return mPos; }
int const&      RedstoneUpdateAfterEvent::getStrength() const { return mStrength; }
bool const&     RedstoneUpdateAfterEvent::getIsFirstTime() const { return mIsFirstTime; }

LL_TYPE_INSTANCE_HOOK(
    RedstoneUpdateEventHook,
    HookPriority::Low,
    CircuitSystem,
    &CircuitSystem::updateBlocks,
    void,
    BlockSource&    region,
    BlockPos const& chunkPos
)
{
    if (!mHasBeenEvaluated) { return; }
    auto& activeComponents = mSceneGraph->mActiveComponentsPerChunk;
    if (activeComponents.empty()) { return; }

    const auto& components = activeComponents.find(chunkPos);
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
                block.mLegacyBlock->onRedstoneUpdate(region, pos, strength, comp->mIsFirstTime);
                LLEventBus.publish(RedstoneUpdateAfterEvent(region, pos, strength, comp->mIsFirstTime));
            }
            comp->mIsFirstTime = false;
        }
    };

    for (auto& item : *components->second.mComponents)
    {
        if (BaseCircuitComponent* comp = item.mComponent; comp && !comp->mRemoved && comp->mNeedsUpdate)
        {
            comp->mNeedsUpdate = false;
            if (comp->isSecondaryPowered()) { secondaryPoweredList.emplace_back(item); }
            else { processComponent(comp, region, item.mPos); }
        }
    }

    for (const auto& item : secondaryPoweredList)
    {
        if (BaseCircuitComponent* comp = item.mComponent; comp) { processComponent(comp, region, item.mPos); }
    }
}

Event_Hook_Factory(RedstoneUpdate, <RedstoneUpdateEventHook>);

} // namespace ila::mc::inline world