#include <mc/network/packet/MobEquipmentPacket.h>
#include <mc/util/Random.h>
#include <mc/util/VariantParameterList.h>
#include <mc/util/VariantParameterListConst.h>
#include <mc/world/ContainerID.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorDefinitionDescriptor.h>
#include <mc/world/actor/Mob.h>
#include <mc/world/actor/ai/goal/PlaceBlockGoal.h>
#include <mc/world/events/gameevents/GameEventRegistry.h>
#include <mc/world/item/ItemStack.h>
#include <mc/world/item/ItemStackBase.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/ILevel.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/BlockChangeContext.h>
#include <mc/world/level/block/BlockType.h>
#include <mc/world/level/dimension/Dimension.h>

// Reconstructed from IDA MCP for ?tick@PlaceBlockGoal@@UEAAXXZ.
// The decompiler JUMPOUT is only the /GS security-cookie failure tail.

void PlaceBlockGoal::tick()
{
    auto& random     = mMob.getRandom();
    auto& definition = *mDefinition;

    auto targetPos = BlockPos { mMob.getPosition() };
    auto xOffset   = definition.mXZRange->rangeMin;
    if (auto const span = definition.mXZRange->rangeMax - xOffset; span > 0)
    {
        xOffset += random.nextInt(span + 1);
    }
    targetPos.x += xOffset;

    auto yOffset = definition.mYRange->rangeMin;
    if (auto const span = definition.mYRange->rangeMax - yOffset; span > 0)
    {
        yOffset += random.nextInt(span + 1);
    }
    targetPos.y += yOffset;

    auto zOffset = definition.mXZRange->rangeMin;
    if (auto const span = definition.mXZRange->rangeMax - zOffset; span > 0)
    {
        zOffset += random.nextInt(span + 1);
    }
    targetPos.z += zOffset;

    auto& region = const_cast<BlockSource&>(mMob.getDimensionBlockSourceConst());

    if (!region.getBlock(targetPos).isAir()) { return; }

    BlockPos belowPos { targetPos.x, targetPos.y - 1, targetPos.z };
    auto&    belowBlock = region.getBlock(belowPos);
    // IDA misnames this helper; the function body is a direct cached mIsSolid load.
    if (belowBlock.isAir() || !belowBlock.mCachedComponentData->mIsSolid) { return; }

    VariantParameterList params {};
    params.mSelf   = &mMob;
    params.mTarget = (mMob.mLevel != nullptr && mMob.mTargetId->rawID != -1)
                         ? mMob.mLevel->fetchEntity(mMob.mTargetId, false)
                         : nullptr;
    params.mBlock  = &targetPos;

    if (!definition.mRandomlyPlaceableBlocks->empty())
    {
        VariantParameterListConst constParams {};
        constParams.mSelf    = params.mSelf;
        constParams.mOther   = params.mOther;
        constParams.mPlayer  = params.mPlayer;
        constParams.mTarget  = params.mTarget;
        constParams.mParent  = params.mParent;
        constParams.mBaby    = params.mBaby;
        constParams.mBlock   = params.mBlock;
        constParams.mDamager = params.mDamager;
        constParams.mHolder  = params.mHolder;

        auto const* randomBlock = _tryGetRandomPlaceBlock(constParams, random);
        if (randomBlock != nullptr)
        {
            BlockChangeContext context {};
            region.setBlock(targetPos, *randomBlock, 3, nullptr, context);
            region.postGameEvent(&mMob, GameEventRegistry::blockPlace(), targetPos, randomBlock);
            ActorDefinitionDescriptor::executeTrigger(mMob, definition.mOnPlace, params);
        }
        return;
    }

    auto const& carriedItem  = mMob.getCarriedItem();
    auto const* carriedBlock = carriedItem.mBlock;
    if (carriedBlock != nullptr && carriedBlock->getBlockType().mayPlace(region, targetPos))
    {
        mMob.setCarriedItem(ItemStack::EMPTY_ITEM());

        MobEquipmentPacket packet {
            mMob.getRuntimeID(), ItemStack::EMPTY_ITEM(), 0, 0, ContainerID::Inventory,
        };
        const_cast<Dimension&>(mMob.getDimensionConst()).sendPacketForEntity(mMob, packet, nullptr);

        BlockChangeContext context {};
        region.setBlock(targetPos, *carriedBlock, 3, nullptr, context);
        region.postGameEvent(&mMob, GameEventRegistry::blockPlace(), targetPos, carriedBlock);
        ActorDefinitionDescriptor::executeTrigger(mMob, definition.mOnPlace, params);
    }
}
