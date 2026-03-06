#pragma once
#include <mc/deps/core/utility/AutomaticID.h>
#include <mc/world/item/ItemStack.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/IBlockSource.h>
#include <mc/world/level/block/ResourceDropsCause.h>

struct ResourceDropsContext {
public:
    ResourceDropsCause  mCause;
    float               mExplosionRadius;
    ItemStack const&    mUsedItem;
    BlockPos const      mBlockPos;
    DimensionType const mDimensionType;
    IBlockSource const& mBlockSource;

public:
    MCAPI int  getMiningLootBonusLevel() const;
    MCAPI bool isUsingSilkTouch() const;
};