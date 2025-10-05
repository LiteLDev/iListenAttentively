#include "ila/event/minecraft/world/actor/MobPlaceBlockEvent.h"
#include "ila/base/Gloabl.h"
#include "ila/patch/VariantParameterList.hpp"
#include <mc/deps/vanilla_components/StateVectorComponent.h>
#include <mc/util/Random.h>
#include <mc/util/Randomize.h>
#include <mc/world/actor/ActorDefinitionDescriptor.h>
#include <mc/world/actor/ai/goal/PlaceBlockGoal.h>
#include <mc/world/item/ItemStack.h>
#include <mc/world/level/Block/Block.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/dimension/Dimension.h>

namespace ila::mc::inline world::inline actor
{

void MobPlaceBlockBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]  = ListTag { pos().x, pos().y, pos().z };
    nbt["block"] = serializePtrObj(block());
}
void MobPlaceBlockBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x = nbt["pos"][0];
    pos().y = nbt["pos"][1];
    pos().z = nbt["pos"][2];
}
BlockPos& MobPlaceBlockBeforeEvent::pos() const { return mPos; }
Block const* MobPlaceBlockBeforeEvent::block() const { return mBlock; }

void MobPlaceBlockAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["pos"]  = ListTag { pos().x, pos().y, pos().z };
    nbt["block"] = serializePtrObj(block());
}
BlockPos const& MobPlaceBlockAfterEvent::pos() const { return mPos; }
Block const* MobPlaceBlockAfterEvent::block() const { return mBlock; }

LL_TYPE_INSTANCE_HOOK(MobPlaceBlockHook, HookPriority::Low, PlaceBlockGoal, &PlaceBlockGoal::$tick, void)
{
    constexpr static auto ramdonPos = [](Randomize& random, int& value, IntRange& ranage) -> void {
        auto min = ranage.mUnk8edd10.as<int>(), max = ranage.mUnkac0553.as<int>();
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
        block.isAir() || !block.mCachedComponentData->mIsSolid)
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
        auto beforeEvent = MobPlaceBlockBeforeEvent {
            mMob,
            targetPos,
            block
        };
        LLEventBus.publish(beforeEvent);
        if (beforeEvent.isCancelled()) { return; }
        _tryPlaceCarriedBlock(region, targetPos, reinterpret_cast<::VariantParameterList&>(params));
        LLEventBus.publish(MobPlaceBlockAfterEvent {
            mMob,
            targetPos,
            block
        });
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
        region.setBlock(targetPos, *randomBlock, 3, nullptr, nullptr);
        std::vector<std::pair<std::string const, std::string const>> eventStack;
        reinterpret_cast<decltype(&ActorDefinitionDescriptor::_executeTrigger)>(
            "40 53 55 56 57 48 81 EC ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 84 24 ?? ?? ?? ?? 49 8B 01"_sig.resolve()
        )(
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

} // namespace ila::mc::inline world::inline actor

// void PlaceBlockGoal::tick() {
//     // Get random number generator
//     Randomize randomize;
//     ILevel* level = this->mMob->mLevel;
//     Random* random = nullptr;
    
//     if (level) {
//         random = level->getThreadRandom();
//     } else {
//         // Fallback to thread-local random
//         random = getThreadLocalRandom();
//     }
    
//     randomize.mRandom.mPointer = random;

//     // Get mob's current position and convert to block coordinates
//     StateVectorComponent* stateVec = this->mMob->mBuiltInComponents.mStateVectorComponent.ptr_;
//     if (!stateVec) {
//         gsl::details::terminate();
//     }

//     Vec3 mobPos = stateVec->mPos;
//     BlockPos currentBlockPos(
//         static_cast<int>(std::floor(mobPos.x)),
//         static_cast<int>(std::floor(mobPos.y)), 
//         static_cast<int>(std::floor(mobPos.z))
//     );

//     // Calculate target position with random offsets within defined ranges
//     BlockPos targetPos = currentBlockPos;
    
//     // Apply X range randomization
//     int xzRangeMin = this->mDefinition.mXZRange.rangeMin;
//     int xzRangeMax = this->mDefinition.mXZRange.rangeMax;
//     if (xzRangeMin < xzRangeMax && random) {
//         targetPos.x += xzRangeMin + random->nextInt(xzRangeMax - xzRangeMin + 1);
//     } else {
//         targetPos.x += xzRangeMin;
//     }

//     // Apply Y range randomization  
//     int yRangeMin = this->mDefinition.mYRange.rangeMin;
//     int yRangeMax = this->mDefinition.mYRange.rangeMax;
//     if (yRangeMin < yRangeMax && random) {
//         targetPos.y += yRangeMin + random->nextInt(yRangeMax - yRangeMin + 1);
//     } else {
//         targetPos.y += yRangeMin;
//     }

//     // Apply Z range randomization (same as X)
//     if (xzRangeMin < xzRangeMax && random) {
//         targetPos.z += xzRangeMin + random->nextInt(xzRangeMax - xzRangeMin + 1);
//     } else {
//         targetPos.z += xzRangeMin;
//     }

//     // Get dimension and block source
//     auto dimension = this->mMob->mDimension.mHandle._Ptr;
//     BlockSource* blockSource = dimension->getBlockSourceFromMainChunkSource();
    
//     // Check if target position is air (can place block here)
//     const Block* targetBlock = blockSource->getBlock(&targetPos);
//     if (!targetBlock || targetBlock->mBlockType.ptr_->mStrHash != BedrockBlockNames::Air.mStrHash) {
//         // Target position is not air, cannot place block here
//         goto cleanup;
//     }

//     // Check if the block below target position is solid (provides support)
//     BlockPos belowPos(targetPos.x, targetPos.y - 1, targetPos.z);
//     const Block* belowBlock = blockSource->getBlock(&belowPos);
    
//     if (!belowBlock || belowBlock->mBlockType.ptr_->mStrHash == BedrockBlockNames::Air.mStrHash || !belowBlock->isSolid()) {
//         // Block below is air or not solid, cannot place block here
//         goto cleanup;
//     }

//     // Setup parameters for block placement
//     VariantParameterList params;
//     params.mSelf = this->mMob;
//     params.mOther = nullptr;
//     params.mPlayer = nullptr;
//     params.mDamager = nullptr;
//     params.mHolder = nullptr;
    
//     // Try to get target entity if exists
//     Actor* target = nullptr;
//     if (this->mMob->mLevel && this->mMob->mTargetId.rawID != -1) {
//         target = this->mMob->mLevel->fetchEntity(this->mMob->mTargetId.rawID);
//     }
//     params.mTarget = target;
//     params.mBlock = &targetPos;

//     // Place either carried block or random block from definition
//     if (this->mDefinition.mRandomlyPlaceableBlocks.empty()) {
//         // No random blocks defined, try to place carried block
//         PlaceBlockGoal::_tryPlaceCarriedBlock(this, blockSource, &targetPos, &params);
//     } else {
//         // Get random block from defined list
//         VariantParameterListConst paramsConst(params);
//         const Block* randomBlock = PlaceBlockGoal::_tryGetRandomPlaceBlock(this, &paramsConst, random);
        
//         if (randomBlock) {
//             // Place the randomly selected block
//             blockSource->setBlock(&targetPos, randomBlock, 3, 0, 0);
            
//             // Post block place game event
//             blockSource->postGameEvent(blockSource, this->mMob, 
//                                      &GameEventRegistry::blockPlace, 
//                                      &targetPos, randomBlock);
            
//             // Execute onPlace trigger
//             std::vector<std::pair<const std::string, const std::string>> eventStack;
//             ActorDefinitionDescriptor::_executeTrigger(this->mMob, 
//                                                      &this->mDefinition.mOnPlace, 
//                                                      &eventStack, 
//                                                      &params);
            
//             // Clean up event stack
//             if (!eventStack.empty()) {
//                 // Destroy and deallocate event stack
//                 // ... (vector cleanup as in original)
//             }
//         }
//     }

// cleanup:
//     // Clean up random resources  
//     if (randomize.mRandom.mControlBlock._Rep) {
//         // Reference counting cleanup
//         if (InterlockedDecrement(&randomize.mRandom.mControlBlock._Rep->_Uses) == 0) {
//             randomize.mRandom.mControlBlock._Rep->_Destroy(randomize.mRandom.mControlBlock._Rep);
//             if (InterlockedDecrement(&randomize.mRandom.mControlBlock._Rep->_Weaks) == 0) {
//                 randomize.mRandom.mControlBlock._Rep->_Delete_this(randomize.mRandom.mControlBlock._Rep);
//             }
//         }
//     }
// }