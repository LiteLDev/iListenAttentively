#include "ila/event/minecraft/world/actor/EndermanLeaveBlockEvent.h"
#include "EndermanLeaveBlockEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/deps/core/math/Vec3.h>
#include <mc/util/Random.h>
#include <mc/world/actor/ai/goal/EndermanLeaveBlockGoal.h>
#include <mc/world/actor/monster/EnderMan.h>
#include <mc/world/events/gameevents/GameEventRegistry.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/block/BedrockBlockNames.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/CachedComponentData.h>
#include <mc/world/level/block/registry/BlockTypeRegistry.h>

namespace ila::mc::inline world::inline actor
{

void EndermanLeaveBlockBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["self"] = serializeRefObj(self());
    nbt["pos"]  = ListTag { pos().x, pos().y, pos().z };
}
void EndermanLeaveBlockBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x = nbt["pos"][0];
    pos().y = nbt["pos"][1];
    pos().z = nbt["pos"][2];
}
EnderMan& EndermanLeaveBlockBeforeEvent::self() const { return static_cast<EnderMan&>(MobEvent::self()); }
BlockPos& EndermanLeaveBlockBeforeEvent::pos() const { return mPos; }

void EndermanLeaveBlockAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["self"] = serializeRefObj(self());
    nbt["pos"]  = ListTag { pos().x, pos().y, pos().z };
}
EnderMan& EndermanLeaveBlockAfterEvent::self() const { return static_cast<EnderMan&>(MobEvent::self()); }
BlockPos const& EndermanLeaveBlockAfterEvent::pos() const { return mPos; }

LL_TYPE_INSTANCE_HOOK(
    EndermanLeaveBlockHook,
    HookPriority::Low,
    EndermanLeaveBlockGoal,
    &EndermanLeaveBlockGoal::$tick,
    void
)
{
    auto& region      = mEnderman.getDimensionBlockSource();
    auto  endermanPos = mEnderman.getPosition();

    auto&    random = mEnderman.getRandom();
    BlockPos placeBlockPos(
        (random.nextFloat() * 2.0f) + (endermanPos.x - 1.0f),
        (random.nextFloat() * 2.0f) + endermanPos.y,
        (random.nextFloat() * 2.0f) + (endermanPos.z - 1.0f)
    );

    if (!region.getBlock(placeBlockPos).isAir()) { return; }

    BlockPos belowPos(placeBlockPos.x, placeBlockPos.y - 1, placeBlockPos.z);
    auto&    belowBlock = region.getBlock(placeBlockPos.add({ 0, -1, 0 }));
    if (belowBlock.isAir() || !belowBlock.mCachedComponentData->mUnkd6c5eb.as<bool>()) { return; }

    auto beforeEvent = EndermanLeaveBlockBeforeEvent(mEnderman, placeBlockPos);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }

    auto& carryingBlock = mEnderman.getCarryingBlock();
    region.setBlock(placeBlockPos, carryingBlock, 3, nullptr, nullptr);
    mEnderman.setCarryingBlock(BlockTypeRegistry::getDefaultBlockState(BedrockBlockNames::Air()));
    region.postGameEvent(&mEnderman, GameEventRegistry::blockPlace(), placeBlockPos, &carryingBlock);

    LLEventBus.publish(EndermanLeaveBlockAfterEvent(mEnderman, placeBlockPos));
}

Event_Hook_Factory(EndermanLeaveBlock, <EndermanLeaveBlockHook>);

} // namespace ila::mc::inline world::inline actor