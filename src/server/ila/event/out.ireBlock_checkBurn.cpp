#include <cstdint>
#include <mc/deps/core/math/IRandom.h>
#include <mc/util/Randomize.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/Weather.h>
#include <mc/world/level/block/BeehiveBlock.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/BlockChangeContext.h>
#include <mc/world/level/block/BlockType.h>
#include <mc/world/level/block/CampfireBlock.h>
#include <mc/world/level/block/FireBlock.h>
#include <mc/world/level/block/VanillaBlockTypeIds.h>
#include <mc/world/level/block/VanillaStates.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/level/storage/GameRuleId.h>
#include <mc/world/level/storage/GameRules.h>

inline static int nextIntInclusive(Randomize& randomize, int min, int max) {
    if (min < max + 1) {
        min += randomize.mRandom->mPointer->nextInt(max + 1 - min);
    }
    return min;
}

void FireBlock::checkBurn(BlockSource& region, BlockPos const& pos, int chance, Randomize& randomize, int age) const {
    Block const& block = region.getBlock(pos);

    BlockType const& blockType = block.getBlockType();

    if (blockType.mNameInfo->mFullName == VanillaBlockTypeIds::Beehive()
        || blockType.mNameInfo->mFullName == VanillaBlockTypeIds::BeeNest()) {
        const_cast<BeehiveBlock&>(static_cast<const BeehiveBlock&>(blockType)).evictAll(region, pos, false);
    }

    if (nextIntInclusive(randomize, 0, chance - 1) < static_cast<int>(block.mDirectData->mBurnOdds)) {
        bool isTnt      = (blockType.mNameInfo->mFullName == VanillaBlockTypeIds::Tnt());
        bool isCampfire = (blockType.mNameInfo->mFullName == VanillaBlockTypeIds::CampFire())
                       || (blockType.mNameInfo->mFullName == VanillaBlockTypeIds::SoulCampfire());

        if (nextIntInclusive(randomize, 0, age + 9) >= 5 || region.getDimension().mWeather->isRainingAt(region, pos)) {
            if (!isCampfire) {
                if (isTnt) {
                    if (region.getLevel().getGameRules().getBool(GameRuleId{19}, false)) {
                        BlockChangeContext context;
                        region.removeBlock(pos, context);
                        return;
                    }
                } else {
                    BlockChangeContext context;
                    region.removeBlock(pos, context);
                }
            }
        } else {
            int newFireAge = age + nextIntInclusive(randomize, 0, 4) / 4;
            if (newFireAge > 15) {
                newFireAge = 15;
            }

            auto fireBlockWithAge = this->mDefaultState->getBlockType().trySetState<bool>(
                VanillaStates::Age(),
                newFireAge,
                this->mDefaultState->getData()
            );

            if (!isCampfire) {
                if (!isTnt) {
                    BlockChangeContext context;
                    region.removeBlock(pos, context);
                    if (this->isValidFireLocation(region, pos) && fireBlockWithAge) {
                        region.setBlock(pos, *fireBlockWithAge, 3, nullptr, context);
                    }
                    return;
                }
            }
        }

        if (isCampfire) {
            CampfireBlock::tryLightFire(region, pos, nullptr);
        }

        if (isTnt) {
            auto tntWithExplode =
                block.getBlockType().trySetState<int>(VanillaStates::ExplodeBit(), true, block.getData());
            (tntWithExplode ? &(*tntWithExplode) : &block)->mBlockType->destroy(region, pos, *(Block*)(this), nullptr);

            if (region.getLevel().getGameRules().getBool(GameRuleId{19}, false)) {
                BlockChangeContext context;
                region.removeBlock(pos, context);
            }
        }
    }
}
