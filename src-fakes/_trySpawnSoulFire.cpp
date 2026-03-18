#include <mc/common/WeakPtr.h>
#include <mc/util/BaseGameVersion.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/BlockChangeContext.h>
#include <mc/world/level/block/BlockType.h>
#include <mc/world/level/block/FireBlock.h>
#include <mc/world/level/block/SoulFireBlock.h>
#include <mc/world/level/block/VanillaBlockTypeIds.h>
#include <mc/world/level/block/registry/BlockTypeRegistry.h>
#include <mc/world/level/storage/LevelData.h>

bool FireBlock::_trySpawnSoulFire(BlockSource& region, BlockPos const& pos) const {
    auto const& bt = region.getBlock(BlockPos{pos.x, pos.y - 1, pos.z}).getBlockType();
    if (bt.mNameInfo->mFullName.get() != VanillaBlockTypeIds::SoulSoil()
        && bt.mNameInfo->mFullName.get() != VanillaBlockTypeIds::SoulSand()) {
        return false;
    }

    WeakPtr<BlockType> soulFireType = BlockTypeRegistry::get().lookupByName(VanillaBlockTypeIds::SoulFire(), true);
    if (!soulFireType) {
        return false;
    }

    if (!soulFireType->mMinRequiredBaseGameVersion->isCompatibleWith(
            region.getLevel().getLevelData().getBaseGameVersion()
        )) {
        return false;
    }

    return region.setBlock(
        pos,
        BlockTypeRegistry::get().getDefaultBlockState(VanillaBlockTypeIds::SoulFire(), true),
        3,
        nullptr,
        BlockChangeContext{false}
    );
}
