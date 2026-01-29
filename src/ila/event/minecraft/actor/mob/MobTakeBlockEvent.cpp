#pragma include_alias("mc/world/events/ActorGriefingBlockEvent.h", "ila/patch/ActorGriefingBlockEvent.hpp")
#include "ila/event/minecraft/actor/mob/MobTakeBlockEvent.h"
#include "ila/base/Gloabl.h"
#include "ila/patch/VariantParameterList.hpp"
#include <ila/patch/ActorGriefingBlockEvent.hpp>
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/entity/ActorEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/deps/ecs/gamerefs_entity/GameRefsEntity.h>
#include <mc/deps/vanilla_components/StateVectorComponent.h>
#include <mc/gameplayhandlers/CoordinatorResult.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/util/IntRange.h>
#include <mc/util/Random.h>
#include <mc/util/Randomize.h>
#include <mc/util/VariantParameterList.h>
#include <mc/world/actor/ActorDefinitionDescriptor.h>
#include <mc/world/actor/ai/goal/TakeBlockGoal.h>
#include <mc/world/events/ActorEventCoordinator.h>
#include <mc/world/events/ActorGameplayEvent.h>
#include <mc/world/events/BlockSourceHandle.h>
#include <mc/world/events/EventRef.h>
#include <mc/world/events/gameevents/GameEventRegistry.h>
#include <mc/world/item/ItemStack.h>
#include <mc/world/level/Block/Block.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/ShapeType.h>
#include <mc/world/level/block/ActorChangeContext.h>
#include <mc/world/level/block/BedrockBlockNames.h>
#include <mc/world/level/block/BlockChangeContext.h>
#include <mc/world/level/block/BlockDescriptor.h>
#include <mc/world/level/block/block_events/BlockRandomTickEvent.h>
#include <mc/world/level/dimension/Dimension.h>

template<>
struct MutableActorGameplayEvent<void>
{
};

namespace ila::mc::inline actor::inline mob
{

void MobTakeBlockBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"] = ListTag { pos().x, pos().y, pos().z };
}
void MobTakeBlockBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x = nbt["pos"][0];
    pos().y = nbt["pos"][1];
    pos().z = nbt["pos"][2];
}
BlockPos& MobTakeBlockBeforeEvent::pos() const { return mPos; }

void MobTakeBlockAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["pos"] = ListTag { pos().x, pos().y, pos().z };
}
BlockPos const& MobTakeBlockAfterEvent::pos() const { return mPos; }

LL_TYPE_INSTANCE_HOOK(MobTakeBlockHook, HookPriority::Low, TakeBlockGoal, &TakeBlockGoal::$tick, void)
{
    using namespace ll::memory_literals;
    constexpr static auto ramdonPos = [](Randomize& random, int& value, IntRange& ranage) -> void {
        auto min = ranage.rangeMin, max = ranage.rangeMax;
        value += min < max && *random.mRandom ? random.mRandom->mPointer->nextInt(max + 1 - min) : min;
    };

    Randomize random { mMob.mLevel->getThreadRandom() };
    auto      targetPos = BlockPos { mMob.mBuiltInComponents->mStateVectorComponent->mPos };

    ramdonPos(random, targetPos.x, mDefinition->mXZRange);
    ramdonPos(random, targetPos.y, mDefinition->mYRange);
    ramdonPos(random, targetPos.z, mDefinition->mXZRange);

    // clang-format off
    auto& region = mMob.mDimension->lock()->getBlockSourceFromMainChunkSource();
    if (
        auto& block = region.getBlock(targetPos);
        !block.isAir() && ( // 这个判断isAir是我自己加的，原版没有这个判断
            mDefinition->mValidBlocks->empty()
            || BlockDescriptor::anyMatch(mDefinition->mValidBlocks, block)
        ) && (
            !mDefinition->mRequiresLineOfSight
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
        if (
            mMob.mLevel->getActorEventCoordinator().sendEvent(
                EventRef<ActorGameplayEvent<CoordinatorResult>> { griefEvent }
            ) == CoordinatorResult::Continue
        ) {
            mMob.setCarriedItem(ItemStack{*block.mBlockType->mDefaultState, 1,nullptr});
            BlockChangeContext context{false};
            context.mContextSource = {ActorChangeContext{&mMob}};
            region.removeBlock(targetPos, context);
            region.postGameEvent(&mMob, GameEventRegistry::blockDestroy(), targetPos, &block);
            ila::patch::VariantParameterList params{
                .mSelf = &mMob,
                .mTarget = mMob.mTargetId->rawID == -1 ? nullptr : mMob.mLevel->fetchEntity(mMob.mTargetId, false),
                .mBlock = &targetPos
            };
            ActorDefinitionDescriptor::executeTrigger(mMob, mDefinition->mOnTake, reinterpret_cast<::VariantParameterList&>(params));
            LLEventBus.publish(MobTakeBlockAfterEvent(mMob, targetPos));
        }
    }
    // clang-format on
}

Event_Hook_Factory(MobTakeBlock, <MobTakeBlockHook>);

} // namespace ila::mc::inline actor::inline mob