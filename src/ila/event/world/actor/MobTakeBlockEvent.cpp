#include "ila/event/world/actor/MobTakeBlockEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/deps/ecs/gamerefs_entity/GameRefsEntity.h>
#include <mc/deps/vanilla_components/StateVectorComponent.h>
#include <mc/gameplayhandlers/ActorGameplayHandler.h>
#include <mc/util/NamedMolangScript.h>
#include <mc/util/Random.h>
#include <mc/world/actor/ActorDefinitionDescriptor.h>
#include <mc/world/actor/ai/goal/TakeBlockGoal.h>
#include <mc/world/events/ActorEventCoordinator.h>
#include <mc/world/events/ActorGriefingBlockEvent.h>
#include <mc/world/events/BlockSourceHandle.h>
#include <mc/world/events/gameevents/GameEventRegistry.h>
#include <mc/world/item/ItemStack.h>
#include <mc/world/level/Block/Block.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/BedrockBlockNames.h>
#include <mc/world/level/block/BlockChangeContext.h>
#include <mc/world/level/block/BlockDescriptor.h>
#include <mc/world/level/block/block_events/BlockRandomTickEvent.h>
#include <mc/world/level/dimension/Dimension.h>

template <>
struct MutableActorGameplayEvent<void> {};

namespace ila::mc::inline world::inline actor {

void MobTakeBlockBeforeEvent::serialize(CompoundTag& nbt) const {
    Cancellable::serialize(nbt);
    nbt["pos"] = ListTag{pos().x, pos().y, pos().z};
}
void MobTakeBlockBeforeEvent::deserialize(CompoundTag const& nbt) {
    Cancellable::deserialize(nbt);
    pos().x = nbt["pos"][0];
    pos().y = nbt["pos"][1];
    pos().z = nbt["pos"][2];
}
BlockPos& MobTakeBlockBeforeEvent::pos() const { return mPos; }

void MobTakeBlockAfterEvent::serialize(CompoundTag& nbt) const {
    ActorEvent::serialize(nbt);
    nbt["pos"] = ListTag{pos().x, pos().y, pos().z};
}
BlockPos const& MobTakeBlockAfterEvent::pos() const { return mPos; }

bool BlockDescriptor_anyMatch(std::vector<BlockDescriptor> const& descriptors, Block const& block) {
    for (auto& des : descriptors) {
        if (des.matches(block)) {
            return true;
        }
    }
    return false;
}

LL_TYPE_INSTANCE_HOOK(MobTakeBlockHook, HookPriority::Low, TakeBlockGoal, &TakeBlockGoal::$tick, void) {
    using namespace ll::memory_literals;
    constexpr static auto ramdonPos = [](Random& random, int& value, IntRange& ranage) -> void {
        auto min = ranage.rangeMin, max = ranage.rangeMax;
        value += min < max && random.nextInt(max + 1 - min);
    };

    Random& random    = mMob.mLevel->getThreadRandom();
    auto    targetPos = BlockPos{*mMob.mBuiltInComponents->mStateVectorComponent->mPos};

    ramdonPos(random, targetPos.x, mXZRange);
    ramdonPos(random, targetPos.y, mYRange);
    ramdonPos(random, targetPos.z, mXZRange);

    // clang-format off
    auto& region = mMob.mDimension->lock()->getBlockSourceFromMainChunkSource();
    if (
        auto& block = region.getBlock(targetPos);
        !block.isAir() && ( // 这个判断isAir是我自己加的，原版没有这个判断
            mValidBlocks->empty()
            || BlockDescriptor_anyMatch(mValidBlocks, block)
        ) && (
            !mRequiresLineOfSight
            || mMob.canSee(targetPos, ShapeType::Collision))
        )
    {
        auto beforeEvent = MobTakeBlockBeforeEvent(mMob, targetPos);
        LLEventBus.publish(beforeEvent);
        if (beforeEvent.isCancelled()) { return; }
        ActorGriefingBlockEvent const& griefEvent {
            mMob.mEntityContext->getWeakRef(),
            &block,
            targetPos,
            std::make_shared<BlockSourceHandle>(region)
        };
        auto& coordinator = mMob.mLevel->getActorEventCoordinator();
        auto* handler = coordinator.mActorGameplayHandler.get();
        if (
           handler && coordinator._processEvent(handler, const_cast<ActorGameplayEvent<CoordinatorResult> const&>(EventRef<ActorGameplayEvent<CoordinatorResult>> { griefEvent }.get())) == CoordinatorResult::Continue
        ) {
            ItemStack item;
            item.reinit(*block.mBlockType->mDefaultState, 1);
            mMob.setCarriedItem(item);
            BlockChangeContext context{};
            context.mContextSource = {ActorChangeContext{&mMob}};
            region.removeBlock(targetPos, context);
            region.postGameEvent(&mMob, GameEventRegistry::blockDestroy(), targetPos, &block);
            VariantParameterList params{};
            params.mSelf = &mMob;
            if (mMob.mTargetId->rawID != -1) params.mTarget = mMob.mLevel->fetchEntity(mMob.mTargetId, false);
            params.mBlock = &targetPos;
            ActorDefinitionDescriptor::executeTrigger(mMob, mOnTake, reinterpret_cast<::VariantParameterList&>(params));
            LLEventBus.publish(MobTakeBlockAfterEvent(mMob, targetPos));
        }
    }
    // clang-format on
}

Event_Hook_Factory(MobTakeBlock, <MobTakeBlockHook>);

} // namespace ila::mc::inline world::inline actor
