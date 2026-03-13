#include <cstdint>
#include <ll/api/base/StdInt.h>
#include <mc/deps/core/math/Random.h>
#include <mc/deps/shared_types/legacy/Difficulty.h>
#include <mc/util/Random.h>
#include <mc/util/Randomize.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/Weather.h>
#include <mc/world/level/biome/Biome.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/BlockChangeContext.h>
#include <mc/world/level/block/BlockProperty.h>
#include <mc/world/level/block/BlockType.h>
#include <mc/world/level/block/FireBlock.h>
#include <mc/world/level/block/FlameOdds.h>
#include <mc/world/level/block/VanillaStates.h>
#include <mc/world/level/block/block_events/BlockQueuedTickEvent.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/level/material/Material.h>
#include <mc/world/level/material/MaterialType.h>
#include <mc/world/level/storage/GameRuleId.h>
#include <mc/world/level/storage/GameRules.h>

void FireBlock::tick(BlockEvents::BlockQueuedTickEvent& eventData) const {
    BlockSource&    region  = eventData.mRegion;
    const BlockPos& firePos = eventData.mPos;
    Random&         random  = eventData.mRandom;

    if (_trySpawnSoulFire(region, firePos)) {
        return;
    }

    Block const& belowBlock = region.getBlock({firePos.x, firePos.y - 1, firePos.z});

    bool infiniBurn =
        (static_cast<uint64>(belowBlock.mBlockType->mProperties) & static_cast<uint64>(BlockProperty::InfiniBurn)) != 0;

    auto infiniburnState = belowBlock.getState<bool>(VanillaStates::InfiniburnBit().mID);
    if (infiniburnState.has_value()) {
        infiniBurn = infiniburnState.value();
    }

    if (!mayPlace(region, firePos)) {
        BlockChangeContext ctx;
        region.removeBlock(firePos, ctx);
        return;
    }

    GameRules& gameRules = region.getLevel().getGameRules();

    if (!gameRules.getBool(GameRuleId(3), false)) {
        _tryAddToTickingQueue(region, firePos, random);
        return;
    }

    if (!gameRules.getBool(GameRuleId(39), true)) {
        BlockChangeContext ctx;
        region.removeBlock(firePos, ctx);
        return;
    }

    Weather* weather = region.getDimension().mWeather.get();

    bool isPrecipitatingNearby = false;

    if (weather->isPrecipitatingAt(region, firePos)) {
        Biome const& biome = region.getBiome(firePos);
        if (biome.getTemperature(region, firePos) > 0.15000001f) {
            isPrecipitatingNearby = true;
        }
    }

    if (!isPrecipitatingNearby) {
        BlockPos testPos(firePos.x + 1, firePos.y, firePos.z);
        if (weather->isPrecipitatingAt(region, testPos)) {
            Biome const& biome = region.getBiome(testPos);
            if (biome.getTemperature(region, testPos) > 0.15000001f) {
                isPrecipitatingNearby = true;
            }
        }
    }

    if (!isPrecipitatingNearby) {
        if (weather->isRainingAt(region, {firePos.x - 1, firePos.y, firePos.z})) {
            isPrecipitatingNearby = true;
        }
    }

    if (!isPrecipitatingNearby) {
        if (weather->isRainingAt(region, {firePos.x, firePos.y, firePos.z - 1})) {
            isPrecipitatingNearby = true;
        }
    }

    if (!isPrecipitatingNearby) {
        if (weather->isRainingAt(region, {firePos.x, firePos.y, firePos.z + 1})) {
            isPrecipitatingNearby = true;
        }
    }

    if (!infiniBurn && region.getDimension().mHasWeather && weather->mRainLevel > 0.2f && isPrecipitatingNearby) {
        BlockChangeContext ctx;
        region.removeBlock(firePos, ctx);
        return;
    }

    Block const& fireBlock = region.getBlock(firePos);
    int          fireAge   = fireBlock.getState<int>(VanillaStates::Age().mID).value_or(0);

    MaterialType belowMaterialType = region.getBlock({firePos.x, firePos.y - 1, firePos.z}).mBlockType->mMaterial.mType;

    if (!infiniBurn && fireAge < 15) {
        fireAge += random.nextInt(3) / 2;

        auto newFireBlock = fireBlock.mBlockType->trySetState<int>(VanillaStates::Age().mID, fireAge, fireBlock.mData);

        BlockChangeContext ctx;
        region.setBlock(firePos, newFireBlock ? *newFireBlock : fireBlock, 1, 0, ctx);
        _tryAddToTickingQueue(region, firePos, random);
    } else {
        _tryAddToTickingQueue(region, firePos, random);
    }

    if (belowMaterialType == MaterialType::Explosive && gameRules.getBool(GameRuleId(20), false)) {
        return;
    }

    BlockPos checkPos(firePos.x, firePos.y - 1, firePos.z);

    if (!isValidFireLocation(region, firePos)) {
        BlockChangeContext ctx;
        region.removeBlock(firePos, ctx);
        return;
    }

    if (region.getLiquidBlock(checkPos).mBlockType->mMaterial.mType == MaterialType::Water) {
        if (!FireBlock::isSolidToppedBlock(region, checkPos)) {
            BlockChangeContext ctx;
            region.removeBlock(firePos, ctx);
            return;
        }

        if (fireAge > 3) {
            BlockChangeContext ctx;
            region.removeBlock(firePos, ctx);
            return;
        }
        return;
    }

    if (region.getBlock(checkPos).mDirectData.get().mFlameOdds == FlameOdds::Never && fireAge == 15
        && !random.nextInt(4)) {
        BlockChangeContext ctx;
        region.removeBlock(firePos, ctx);
        return;
    }

    bool isHumid = region.getBiome(firePos).isHumid();

    int humidityOffset = isHumid ? -50 : 0;

    Randomize randomize(random);

    int horizontalChance = humidityOffset + 300;
    checkBurn(region, {firePos.x + 1, firePos.y, firePos.z}, horizontalChance, randomize, fireAge, firePos);
    checkBurn(region, {firePos.x - 1, firePos.y, firePos.z}, horizontalChance, randomize, fireAge, firePos);
    checkBurn(region, {firePos.x, firePos.y - 1, firePos.z}, humidityOffset + 250, randomize, fireAge, firePos);
    checkBurn(region, {firePos.x, firePos.y + 1, firePos.z}, humidityOffset + 250, randomize, fireAge, firePos);
    checkBurn(region, {firePos.x, firePos.y, firePos.z - 1}, horizontalChance, randomize, fireAge, firePos);
    checkBurn(region, {firePos.x, firePos.y, firePos.z + 1}, horizontalChance, randomize, fireAge, firePos);

    for (int xx = -1; xx <= 1; ++xx) {
        for (int zz = -1; zz <= 1; ++zz) {
            for (int yy = -1; yy <= 4; ++yy) {
                if (xx == 0 && yy == 0 && zz == 0) {
                    continue;
                }

                int spreadRate = 100;
                if (yy > 1) {
                    spreadRate = 100 * yy;
                }

                BlockPos spreadPos(firePos.x + xx, firePos.y + yy, firePos.z + zz);
                float    fireOdds = getFireOdds(region, spreadPos);

                if (fireOdds > 0.0f) {
                    // f(n) = 40 + 7 * n * (1 - min(1, max(0, |n-1.5| - 1.5) / 1.5))
                    int   n            = static_cast<int>(region.getLevel().getDifficulty());
                    float spreadChance = (fireOdds + (n >= 0 && n <= 3) ? (40 + 7 * n) : 40) / (float)(fireAge + 30);
                    if (isHumid) {
                        spreadChance *= 0.5f;
                    }

                    if (spreadChance > 0.0f) {
                        float randomValue =
                            (double)(int)random.mRandom.get().mObject._genRandInt32() * 2.328306436538696e-10;

                        if (randomValue * (float)spreadRate <= spreadChance) {
                            bool isPrecipitating = false;

                            if (weather->isPrecipitatingAt(region, spreadPos)) {
                                if (region.getBiome(spreadPos).getTemperature(region, spreadPos) > 0.15000001f) {
                                    isPrecipitating = true;
                                }
                            }

                            if (!isPrecipitating) {
                                BlockPos checkPos1(spreadPos.x - 1, spreadPos.y, spreadPos.z);
                                if (weather->isPrecipitatingAt(region, checkPos1)) {
                                    if (region.getBiome(checkPos1).getTemperature(region, checkPos1) > 0.15000001f) {
                                        isPrecipitating = true;
                                    }
                                }
                            }

                            if (!isPrecipitating) {
                                BlockPos checkPos2(spreadPos.x + 1, spreadPos.y, spreadPos.z);
                                if (weather->isPrecipitatingAt(region, checkPos2)) {
                                    if (region.getBiome(checkPos2).getTemperature(region, checkPos2) > 0.15000001f) {
                                        isPrecipitating = true;
                                    }
                                }
                            }

                            if (!isPrecipitating) {
                                BlockPos checkPos3(spreadPos.x, spreadPos.y, spreadPos.z - 1);
                                if (weather->isPrecipitatingAt(region, checkPos3)) {
                                    if (region.getBiome(checkPos3).getTemperature(region, checkPos3) > 0.15000001f) {
                                        isPrecipitating = true;
                                    }
                                }
                            }

                            if (!isPrecipitating) {
                                BlockPos checkPos4(spreadPos.x, spreadPos.y, spreadPos.z + 1);
                                if (weather->isPrecipitatingAt(region, checkPos4)) {
                                    if (region.getBiome(checkPos4).getTemperature(region, checkPos4) > 0.15000001f) {
                                        isPrecipitating = true;
                                    }
                                }
                            }

                            if (!region.getDimension().mHasWeather || weather->mRainLevel <= 0.2f || !isPrecipitating) {
                                BlockChangeContext ctx;
                                region.setBlock(spreadPos, fireBlock, 3, 0, ctx);
                            }
                        }
                    }
                }
            }
        }
    }
}
