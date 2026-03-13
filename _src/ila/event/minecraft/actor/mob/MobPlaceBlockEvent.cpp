#include "ila/event/actor/mob/MobPlaceBlockEvent.h"
#include "ila/base/Gloabl.h"
#include "ila/patch/VariantParameterList.hpp"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/entity/ActorEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/deps/vanilla_components/StateVectorComponent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/network/packet/MobEquipmentPacket.h>
#include <mc/util/IntRange.h>
#include <mc/util/Random.h>
#include <mc/util/Randomize.h>
#include <mc/util/VariantParameterList.h>
#include <mc/world/ContainerID.h>
#include <mc/world/actor/ActorDefinitionDescriptor.h>
#include <mc/world/actor/ActorFilterGroup.h>
#include <mc/world/actor/ai/goal/PlaceBlockGoal.h>
#include <mc/world/events/gameevents/GameEventRegistry.h>
#include <mc/world/item/ItemStack.h>
#include <mc/world/level/Block/Block.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/ActorChangeContext.h>
#include <mc/world/level/block/BedrockBlockNames.h>
#include <mc/world/level/block/BlockChangeContext.h>
#include <mc/world/level/dimension/Dimension.h>
#include <string>
#include <utility>
#include <vector>

namespace ila::mc::inline actor::inline mob
{

void MobPlaceBlockBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]   = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["block"] = serializePtrObj(mBlock);
}
void MobPlaceBlockBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mPos.x = nbt["pos"][0];
    mPos.y = nbt["pos"][1];
    mPos.z = nbt["pos"][2];
}

void MobPlaceBlockAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["pos"]   = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["block"] = serializePtrObj(mBlock);
}

LL_TYPE_INSTANCE_HOOK(MobPlaceBlockHook, HookPriority::Low, PlaceBlockGoal, &PlaceBlockGoal::$tick, void)
{
    constexpr static auto ramdonPos = [](Randomize& random, int& value, IntRange& ranage) -> void {
        auto min = ranage.rangeMin, max = ranage.rangeMax;
        value += min < max && *random.mRandom ? random.mRandom->mPointer->nextInt(max + 1 - min) : min;
    };

    Randomize random { mMob.mLevel->getThreadRandom() };
    auto      targetPos = BlockPos { mMob.mBuiltInComponents->mStateVectorComponent->mPos };

    ramdonPos(random, targetPos.x, mDefinition->mXZRange);
    ramdonPos(random, targetPos.y, mDefinition->mYRange);
    ramdonPos(random, targetPos.z, mDefinition->mXZRange);

    auto& region = mMob.mDimension->lock()->getBlockSourceFromMainChunkSource();
    if (!region.getBlock(targetPos).isAir()) { return; }
    if (auto& block = region.getBlock(targetPos.add({ 0, -1, 0 }));
        (block.getTypeName() == BedrockBlockNames::Air().getString())
        || !block.mCachedComponentData->mIsSolid)
    {
        return;
    }

    // clang-format off
    VariantParameterList params {
        .mSelf   = &mMob,
        .mTarget = mMob.mLevel && mMob.mTargetId->rawID != -1
            ? mMob.mLevel->fetchEntity(mMob.mTargetId, false)
            : nullptr,
        .mBlock  = &targetPos
    };
    if (mDefinition->mRandomlyPlaceableBlocks->empty())
    {
        auto& block = mMob.getCarriedItem().mBlock;
        if (block->mBlockType->mayPlace(region, targetPos)) {
            auto beforeEvent = MobPlaceBlockBeforeEvent {
                mMob,
                targetPos,
                block
            };
            LLEventBus.publish(beforeEvent);
            if (beforeEvent.isCancelled()) { return; }
            if (block->mBlockType->mayPlace(region, targetPos)) {
                mMob.setCarriedItem(ItemStack::EMPTY_ITEM());
                MobEquipmentPacket{
                    mMob.getRuntimeID(),
                    ItemStack::EMPTY_ITEM(),
                    0,
                    0,
                    ContainerID::Inventory
                }.sendTo(mMob);
                BlockChangeContext context{false};
                context.mContextSource = ActorChangeContext{&mMob};
                region.setBlock(targetPos, *block, 3, nullptr, context);
                region.postGameEvent(&mMob, GameEventRegistry::blockPlace(), targetPos,block);
                std::vector<std::pair<std::string const, std::string const>> stack;
                ActorDefinitionDescriptor::_executeTrigger(mMob, mDefinition->mOnPlace, stack, reinterpret_cast<::VariantParameterList&>(params));
                LLEventBus.publish(MobPlaceBlockAfterEvent {
                    mMob,
                    targetPos,
                    block
                });
            }
        }
    } else if (
        auto* randomBlock = _tryGetRandomPlaceBlock(
            reinterpret_cast<VariantParameterListConst&>(params),
            static_cast<Random&>(**random.mRandom)
        ); randomBlock
    ) {
        auto beforeEvent = MobPlaceBlockBeforeEvent {
            mMob,
            targetPos,
            randomBlock
        };
        LLEventBus.publish(beforeEvent);
        if (beforeEvent.isCancelled()) { return; }
        region.setBlock(targetPos, *randomBlock, 3, nullptr, nullptr, BlockChangeContext{false});
        std::vector<std::pair<std::string const, std::string const>> eventStack;
        ActorDefinitionDescriptor::_executeTrigger(
            mMob,
            mDefinition->mOnPlace,
            eventStack,
            reinterpret_cast<::VariantParameterList&>(params)
        );
        LLEventBus.publish(MobPlaceBlockAfterEvent {
            mMob,
            targetPos,
            randomBlock
        });
    }
    // clang-format on
}

Event_Hook_Factory(MobPlaceBlock, <MobPlaceBlockHook>);

} // namespace ila::mc::inline actor::inline mob