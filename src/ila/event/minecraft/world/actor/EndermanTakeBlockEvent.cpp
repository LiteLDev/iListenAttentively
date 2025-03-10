#include "ila/event/minecraft/world/actor/EndermanTakeBlockEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/ecs/gamerefs_entity/GameRefsEntity.h>
#include <mc/world/actor/ai/goal/EndermanTakeBlockGoal.h>
#include <mc/world/events/ActorEventCoordinator.h>
#include <mc/world/events/BlockSourceHandle.h>
#include <mc/world/events/gameevents/GameEventRegistry.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/BedrockBlockNames.h>
#include <mc/world/level/block/registry/BlockTypeRegistry.h>


BlockSourceHandle::BlockSourceHandle() = default;

namespace ila::mc::inline world::inline actor
{

void EndermanTakeBlockBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["self"] = serializeRefObj(self());
    nbt["pos"]  = ListTag { pos().x, pos().y, pos().z };
}
void EndermanTakeBlockBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x = nbt["pos"][0];
    pos().y = nbt["pos"][1];
    pos().z = nbt["pos"][2];
}
EnderMan& EndermanTakeBlockBeforeEvent::self() const { return static_cast<EnderMan&>(MobEvent::self()); }
BlockPos& EndermanTakeBlockBeforeEvent::pos() const { return mPos; }

void EndermanTakeBlockAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["self"] = serializeRefObj(self());
    nbt["pos"]  = ListTag { pos().x, pos().y, pos().z };
}
EnderMan&       EndermanTakeBlockAfterEvent::self() const { return static_cast<EnderMan&>(MobEvent::self()); }
BlockPos const& EndermanTakeBlockAfterEvent::pos() const { return mPos; }

LL_TYPE_INSTANCE_HOOK(
    EndermanTakeBlockHook,
    HookPriority::Normal,
    EndermanTakeBlockGoal,
    &EndermanTakeBlockGoal::$tick,
    void
)
{
    auto& region    = mEnderman.getDimensionBlockSource();
    auto  randomPos = getRandomNearbyBlockPos(mEnderman.getPosition());
    auto& block     = region.getBlock(randomPos);

    if (!EnderMan::mMayTake().contains(&block)) { return; }

    auto beforeEvent = EndermanTakeBlockBeforeEvent(mEnderman, randomPos);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }

    // clang-format off
    constexpr static auto makeBlockSourceHandle = [](BlockSource& region) -> std::shared_ptr<BlockSourceHandle> {
        auto blockSourceHandle = std::make_shared<BlockSourceHandle>();
        blockSourceHandle->mUnk525e9e.as<BlockSource*>() = &region;
        return blockSourceHandle;
    };
    
    auto const& actorGriefingBlockEvent = ActorGriefingBlockEvent {
        mEnderman.getWeakEntity(),
        &block,
        randomPos,
        makeBlockSourceHandle(region)
    };
    // clang-format on

    auto event = mEnderman.getLevel().getActorEventCoordinator().sendEvent(
        EventRef<ActorGameplayEvent<CoordinatorResult>>(actorGriefingBlockEvent)
    );
    if (event == CoordinatorResult::Cancel) { return; }

    mEnderman.setCarryingBlock(block);

    region.setBlock(
        randomPos,
        BlockTypeRegistry::getDefaultBlockState(BedrockBlockNames::Air()),
        3,
        nullptr,
        nullptr
    );

    region.postGameEvent(&mEnderman, GameEventRegistry::blockDestroy(), randomPos, &block);

    LLEventBus.publish(EndermanTakeBlockAfterEvent(mEnderman, randomPos));
}

Event_Hook_Factory(EndermanTakeBlock, <EndermanTakeBlockHook>);

} // namespace ila::mc::inline world::inline actor