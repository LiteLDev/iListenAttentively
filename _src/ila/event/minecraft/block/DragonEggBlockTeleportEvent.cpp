#include "ila/event/block/DragonEggBlockTeleportEvent.h"
#include "ila/base/Gloabl.h"
#include <cmath>
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/world/WorldEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/deps/core/math/Random.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/shared_types/legacy/LevelEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/util/Random.h>
#include <mc/world/events/gameevents/GameEventRegistry.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/BedrockBlockNames.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/BlockChangeContext.h>
#include <mc/world/level/block/DragonEggBlock.h>
#include <mc/world/level/block/VanillaBlockTypeIds.h>
#include <mc/world/level/block/registry/BlockTypeRegistry.h>


namespace ila::mc::inline block
{

void DragonEggBlockTeleportBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]       = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["dimId"]     = getDimensionName(blockSource());
    nbt["random"]    = serializeRefObj(mRandom);
    nbt["targetPos"] = ListTag { mTargetPos.x, mTargetPos.y, mTargetPos.z };
}
void DragonEggBlockTeleportBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mTargetPos.x = nbt["targetPos"][0];
    mTargetPos.y = nbt["targetPos"][1];
    mTargetPos.z = nbt["targetPos"][2];
}

void DragonEggBlockTeleportAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]       = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["dimId"]     = getDimensionName(blockSource());
    nbt["random"]    = serializeRefObj(mRandom);
    nbt["targetPos"] = ListTag { mTargetPos.x, mTargetPos.y, mTargetPos.z };
}

LL_STATIC_HOOK(
    DragonEggBlockTeleportEventHook,
    HookPriority::Low,
    &DragonEggBlock::_attemptTeleport,
    void,
    BlockSource&    pRegion,
    Random&         pRandom,
    BlockPos const& pPos
)
{
    auto& level = pRegion.getLevel();
    if (level.isClientSide()) { return; }

    int      attemptCount    = 0;
    auto&    randomGenerator = pRandom.mRandom;
    BlockPos targetPos;

    // clang-format off
    while (true)
    {
        int verticalOffset = (randomGenerator->mObject._genRandInt32() & 7) - (randomGenerator->mObject._genRandInt32() & 7);
        targetPos.x = ((randomGenerator->mObject._genRandInt32() & 0xF) - (randomGenerator->mObject._genRandInt32() & 0xF)) + pPos.x;
        targetPos.z = (randomGenerator->mObject._genRandInt32() & 0xF) + pPos.z - (randomGenerator->mObject._genRandInt32() & 0xF);
        targetPos.y = (verticalOffset > pRegion.getMaxHeight() ? 0 : verticalOffset) + pPos.y;

        if (pRegion.getBlock({targetPos.x, targetPos.y, targetPos.z}).isAir()) { break; }
        if (++attemptCount >= 1000) { return; }
    }
    // clang-format on

    auto beforeEvent = DragonEggBlockTeleportBeforeEvent(pRegion, pPos, pRandom, targetPos);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }

    pRegion.postGameEvent(nullptr, GameEventRegistry::teleport(), pPos, nullptr);
    auto x = pPos.x - targetPos.x;
    auto y = pPos.y - targetPos.y;
    auto z = pPos.z - targetPos.z;
    level.broadcastLocalEvent(
        pRegion,
        SharedTypes::Legacy::LevelEvent::ParticlesDragonEgg,
        pPos,
        abs(z) | ((abs(y) | (((((x >> 31) | (2 * ((y >> 31) | (2 * (z >> 31))))) << 8) | abs(x)) << 8)) << 8)
    );
    pRegion.setBlock(
        targetPos,
        BlockTypeRegistry::get().getDefaultBlockState(VanillaBlockTypeIds::DragonEgg(), true),
        3 /* BlockUpdateFlag::All */,
        nullptr,
        BlockChangeContext { false }
    );
    pRegion.removeBlock(pPos, BlockChangeContext { false });
    LLEventBus.publish(DragonEggBlockTeleportAfterEvent(pRegion, pPos, pRandom, targetPos));
}

Event_Hook_Factory(DragonEggBlockTeleport, <DragonEggBlockTeleportEventHook>);

} // namespace ila::mc::inline block