#pragma once
#include <mc/_HeaderOutputPredefine.h>
#include <mc/util/Random.h>
#include <mc/world/item/ItemStack.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/IBlockSource.h>
#include <mc/world/level/block/ResourceDropsCause.h>
#include <mc/world/level/dimension/DimensionType.h>

struct ResourceDropsContext {
public:
    ResourceDropsCause  mCause;
    float               mExplosionRadius;
    ItemStack const&    mUsedItem;
    BlockPos const      mBlockPos;
    DimensionType const mDimensionType;
    IBlockSource const& mBlockSource;

public:
    MCAPI BlockActor const* getBlockActor() const;
    MCAPI ILevel&           getLevel() const;
    MCAPI int               getMiningLootBonusLevel() const;
    MCAPI Random&           getRandom() const;
    MCAPI bool              isUsingSilkTouch() const;

public:
    MCAPI static ResourceDropsContext
    fromExplosion(IBlockSource const& region, float explosionRadius, BlockPos const& position);
    MCAPI static ResourceDropsContext
    fromLootResolver(IBlockSource const& region, BlockPos const& position, ItemStack const& usedItem);
    MCAPI static ResourceDropsContext fromOtherCause(IBlockSource const& region, BlockPos const& position);
    MCAPI static ResourceDropsContext
    fromPlayerMining(IBlockSource const& region, BlockPos const& position, ItemStack const& usedItem);
    MCAPI static ResourceDropsContext fromProjectileHit(IBlockSource const& region, BlockPos const& position);
};
