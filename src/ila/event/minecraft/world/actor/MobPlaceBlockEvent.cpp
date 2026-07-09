#include "ila/event/minecraft/world/actor/MobPlaceBlockEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/entity/ActorEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/deps/nbt/CompoundTag.h>
#include <mc/deps/nbt/ListTag.h>
#include <mc/deps/vanilla_components/StateVectorComponent.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/network/packet/MobEquipmentPacket.h>
#include <mc/util/IntRange.h>
#include <mc/util/NamedMolangScript.h>
#include <mc/util/Random.h>
#include <mc/util/VariantParameterList.h>
#include <mc/util/VariantParameterListConst.h>
#include <mc/world/ContainerID.h>
#include <mc/world/actor/ActorDefinitionDescriptor.h>
#include <mc/world/actor/ActorFilterGroup.h>
#include <mc/world/actor/ai/goal/PlaceBlockGoal.h>
#include <mc/world/events/gameevents/GameEventRegistry.h>
#include <mc/world/filters/FilterContext.h>
#include <mc/world/item/ItemStack.h>
#include <mc/world/level/Block/Block.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/BlockChangeContext.h>
#include <mc/world/level/dimension/Dimension.h>
#include <string>
#include <utility>
#include <vector>

namespace ila::mc::inline world::inline actor
{

void MobPlaceBlockBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]   = ListTag { pos().x, pos().y, pos().z };
    nbt["block"] = serializePtrObj(block());
}
void MobPlaceBlockBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x = nbt["pos"][0];
    pos().y = nbt["pos"][1];
    pos().z = nbt["pos"][2];
}
BlockPos&    MobPlaceBlockBeforeEvent::pos() const { return mPos; }
Block const* MobPlaceBlockBeforeEvent::block() const { return mBlock; }

void MobPlaceBlockAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["pos"]   = ListTag { pos().x, pos().y, pos().z };
    nbt["block"] = serializePtrObj(block());
}
BlockPos const& MobPlaceBlockAfterEvent::pos() const { return mPos; }
Block const*    MobPlaceBlockAfterEvent::block() const { return mBlock; }

LL_TYPE_INSTANCE_HOOK(MobPlaceBlockHook, HookPriority::Low, PlaceBlockGoal, &PlaceBlockGoal::$tick, void)
{
    constexpr static auto ramdonPos = [](Random& random, int& value, IntRange& ranage) -> void {
        auto min = ranage.rangeMin, max = ranage.rangeMax;
        value += min < max && random.nextInt(max + 1 - min);
    };

    auto& random    = mMob.getRandom();
    auto  targetPos = BlockPos { mMob.getPosition() };

    ramdonPos(random, targetPos.x, mDefinition->mXZRange);
    ramdonPos(random, targetPos.y, mDefinition->mYRange);
    ramdonPos(random, targetPos.z, mDefinition->mXZRange);

    auto& region = mMob.getDimension().getBlockSourceFromMainChunkSource();
    if (!region.getBlock(targetPos).isAir()) { return; }
    if (auto& block = region.getBlock(targetPos.add({ 0, -1, 0 }));
        block.isAir() || !block.mCachedComponentData->mIsSolid)
    {
        return;
    }

    // clang-format off
    VariantParameterList params {};
    params.mSelf   = &mMob;
    if (mMob.mLevel != nullptr && mMob.mTargetId->rawID != -1) {
        params.mTarget = mMob.mLevel->fetchEntity(mMob.mTargetId, false);
    }
    params.mBlock  = &targetPos;
    if (mDefinition->mRandomlyPlaceableBlocks->empty())
    {
        if (auto& item = mMob.getCarriedItem(); item) {
            if (auto* block = item.mBlock; block && !block->isAir() && block->mBlockType->mayPlace(region, targetPos)) {
                auto beforeEvent = MobPlaceBlockBeforeEvent {
                    mMob,
                    targetPos,
                    block
                };
                LLEventBus.publish(beforeEvent);
                if (beforeEvent.isCancelled()) { return; }
                mMob.setCarriedItem(ItemStack::EMPTY_ITEM());
                MobEquipmentPacket{MobEquipmentPacketPayload{
                    mMob.getRuntimeID(),
                    ItemStack::EMPTY_ITEM(),
                    0,
                    0,
                    ContainerID::Inventory
                }}.sendTo(mMob);
                BlockChangeContext context{};
                region.setBlock(targetPos, *block, 3, nullptr, context);
                region.postGameEvent(&mMob, GameEventRegistry::blockPlace(), targetPos, block);
                std::vector<std::pair<std::string const, std::string const>> stack;
                ActorDefinitionDescriptor::executeTrigger(mMob, mDefinition->mOnPlace, params);
                LLEventBus.publish(MobPlaceBlockAfterEvent {
                    mMob,
                    targetPos,
                    block
                });
            }
        }
    } else if (
        auto* randomBlock = thisFor<PlaceBlockGoal>()->_tryGetRandomPlaceBlock(
            static_cast<VariantParameterListConst>(params),
            random
        ); randomBlock
    ) {
        auto beforeEvent = MobPlaceBlockBeforeEvent {
            mMob,
            targetPos,
            randomBlock
        };
        LLEventBus.publish(beforeEvent);
        if (beforeEvent.isCancelled()) { return; }
        BlockChangeContext context{};
        region.setBlock(targetPos, *randomBlock, 3, nullptr, context);
        region.postGameEvent(&mMob, GameEventRegistry::blockPlace(), targetPos, randomBlock);
        std::vector<std::pair<std::string const, std::string const>> eventStack;
         ActorDefinitionDescriptor::executeTrigger(mMob, mDefinition->mOnPlace, params);
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
