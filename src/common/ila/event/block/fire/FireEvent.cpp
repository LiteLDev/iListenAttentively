#include "ila/event/block/fire/FireEvent.h"
#include "ila/base/Gloabl.i.h"
#include "ila/event/block/fire/FireAgeEvent.h"
#include "ila/event/block/fire/FireRemoveEvent.h"
#include "ila/event/block/fire/FireSpreadEvent.h"
#include "ila/event/block/fire/SoulFireSpawnEvent.h"
#include <algorithm>
#include <array>
#include <functional>
#include <ll/api/event/world/WorldEvent.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/reflection/Serialization.h>
#include <ll/api/utils/ErrorUtils.h>
#include <mc/nbt/CompoundTag.h>
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
#include <mc/world/level/block/FireBlock.h>
#include <mc/world/level/block/FlameOdds.h>
#include <mc/world/level/block/VanillaStates.h>
#include <mc/world/level/block/block_events/BlockQueuedTickEvent.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/level/levelgen/structure/BoundingBox.h>
#include <mc/world/level/material/Material.h>
#include <mc/world/level/material/MaterialType.h>
#include <mc/world/level/storage/GameRuleId.h>
#include <mc/world/level/storage/GameRules.h>
#include <mc/world/level/storage/LevelData.h>
#include <utility>


namespace ila::block::inline fire {

void FireEvent::serialize(CompoundTag& nbt) const {
    WorldEvent::serialize(nbt);
    reflection::serialize_to(nbt["pos"], mPos).value();
}

LL_TYPE_INSTANCE_HOOK(
    FireEventHook1,
    HookPriority::Low,
    FireBlock,
    &FireBlock::tick,
    void,
    BlockEvents::BlockQueuedTickEvent& eventData
) {
    auto&     region         = eventData.mRegion;
    auto&     pos            = *eventData.mPos;
    auto&     random         = eventData.mRandom;
    auto&     gameRules      = *region.mLevel.mLevelData->get()->mGameRules;
    auto&     weather        = *region.mDimension.mWeather;
    auto&     fireBlock      = region.getBlock(pos);
    auto      belowPos       = pos.add({0, -1, 0});
    auto&     belowBlock     = region.getBlock(belowPos);
    auto      fireAge        = fireBlock.getState<int>(VanillaStates::Age()).value_or(0);
    auto      isHumid        = region.getBiome(pos).isHumid();
    auto      humidityOffset = isHumid ? -50 : 0;
    Randomize randomize(random);

    {
        auto event = SoulFireSpawningEvent{region, pos};
        getLLEventBus().publish(event);
        if (!event.isCancelled() && _trySpawnSoulFire(region, pos)) {
            return getLLEventBus().publish(SoulFireSpawnedEvent{region, pos});
        }
    }

    auto infiniBurn = [&]() {
        if (auto state = belowBlock.getState<bool>(VanillaStates::InfiniburnBit()); state) return *state;
        return static_cast<bool>(
            std::to_underlying(belowBlock.mBlockType->mProperties) & std::to_underlying(BlockProperty::InfiniBurn)
        );
    }();

    if (!mayPlace(region, pos)) {
        auto event = FireRemovingEvent{region, pos, FireRemoveEvent::Reason::InvalidPos};
        getLLEventBus().publish(event);
        if (!event.isCancelled()) {
            region.removeBlock(pos, BlockChangeContext{false});
            return getLLEventBus().publish(FireRemovedEvent{region, pos, FireRemoveEvent::Reason::InvalidPos});
        }
    }

    if (!gameRules.getBool(GameRuleId{std::to_underlying(GameRules::GameRulesIndex::DoFireTick)}, false)) {
        return _tryAddToTickingQueue(region, pos, random);
    }

    if (!gameRules.getBool(GameRuleId(std::to_underlying(GameRules::GameRulesIndex::AllowDestructiveObjects)), true)) {
        auto event = FireRemovingEvent{region, pos, FireRemoveEvent::Reason::GameRule};
        getLLEventBus().publish(event);
        if (!event.isCancelled()) {
            region.removeBlock(pos, BlockChangeContext{false});
            return getLLEventBus().publish(FireRemovedEvent{region, pos, FireRemoveEvent::Reason::GameRule});
        }
    }

    // clang-format off
    static std::array<std::function<bool(BlockSource& region, Weather& weather, BlockPos const& pos)>, 5> vscNearbyRainfallCheck = {
        [](BlockSource& region, Weather& weather, BlockPos const& pos) {
            return weather.isPrecipitatingAt(region, pos)
            && region.getBiome(pos).getTemperature(region, pos) > 0.15000001f;
        }, 
        [](BlockSource& region, Weather& weather, BlockPos const& pos) {
            auto nearby = pos.add({1, 0, 0});
            return weather.isPrecipitatingAt(region, nearby)
            && region.getBiome(nearby).getTemperature(region, nearby) > 0.15000001f;
        },
        [](BlockSource& region, Weather& weather, BlockPos const& pos) {
            return weather.isRainingAt(region, pos.add({-1, 0, 0}));
        },
        [](BlockSource& region, Weather& weather, BlockPos const& pos) {
            return weather.isRainingAt(region, pos.add({0, 0, -1}));
        },
        [](BlockSource& region, Weather& weather, BlockPos const& pos) {
            return weather.isRainingAt(region, pos.add({0, 0, 1}));
        }
    };
    // clang-format on

    if (!infiniBurn && region.mDimension.mHasWeather && weather.mRainLevel > 0.2f
        && std::ranges::any_of(vscNearbyRainfallCheck, [&](auto&& check) { return check(region, weather, pos); })) {
        auto event = FireRemovingEvent{region, pos, FireRemoveEvent::Reason::Rain};
        getLLEventBus().publish(event);
        if (!event.isCancelled()) {
            region.removeBlock(pos, BlockChangeContext{false});
            return getLLEventBus().publish(FireRemovedEvent{region, pos, FireRemoveEvent::Reason::Rain});
        }
    }

    bool doSpread = false;

    if (infiniBurn) {
        _tryAddToTickingQueue(region, pos, random);
        doSpread = true;
    } else {
        if (fireAge < 15) {
            auto newAge = fireAge + random.nextInt(3) / 2;

            auto event = FireAgingEvent{region, pos, fireAge, newAge};
            getLLEventBus().publish(event);
            if (!event.isCancelled() && event.newAge() != fireAge) {
                region.setBlock(
                    pos,
                    fireBlock.mBlockType->trySetState<int>(VanillaStates::Age(), newAge, fireBlock.mData)
                        .value_or<Block const>(fireBlock),
                    1,
                    0,
                    BlockChangeContext{false}
                );
                std::swap(fireAge, newAge);
                getLLEventBus().publish(FireAgedEvent{region, pos, newAge, fireAge});
            }
        }
        _tryAddToTickingQueue(region, pos, random);

        bool removeFire   = false;
        bool skipAgeCheck = false;

        if (belowBlock.mBlockType->mMaterial.mType != MaterialType::Explosive
            || gameRules.getBool(GameRuleId{std::to_underlying(GameRules::GameRulesIndex::DoTntExplode)}, false)) {
            bool valid = this->isValidFireLocation(region, pos);
            if (!valid) {
                if (!isSolidToppedBlock(region, belowPos)) {
                    removeFire = true;
                }
            } else {
                if (region.getLiquidBlock(belowPos).mBlockType->mMaterial.mType == MaterialType::Water) {
                    if (!isSolidToppedBlock(region, belowPos)) {
                        removeFire = true;
                    }
                } else {
                    if (belowBlock.mDirectData->mFlameOdds == FlameOdds::Never && fireAge == 15 && !random.nextInt(4)) {
                        removeFire = true;
                    } else {
                        skipAgeCheck = true;
                    }
                }
            }
        }

        if (removeFire) {
            region.removeBlock(pos, BlockChangeContext{false});
            return;
        }

        if (!skipAgeCheck && fireAge <= 3) return;

        doSpread = true;
    }

    if (!doSpread) return;

    auto horizontalChance = humidityOffset + 300;
    checkBurn(region, pos.add({1, 0, 0}), horizontalChance, randomize, fireAge, pos);
    checkBurn(region, pos.add({-1, 0, 0}), horizontalChance, randomize, fireAge, pos);
    checkBurn(region, pos.add({0, -1, 0}), humidityOffset + 250, randomize, fireAge, pos);
    checkBurn(region, pos.add({0, 1, 0}), humidityOffset + 250, randomize, fireAge, pos);
    checkBurn(region, pos.add({0, 0, -1}), horizontalChance, randomize, fireAge, pos);
    checkBurn(region, pos.add({0, 0, 1}), horizontalChance, randomize, fireAge, pos);

    for (auto spreadPos : BoundingBox{pos.add(-1), pos.add({1, 4, 1})}.forEachPos()) {
        if (spreadPos == pos) continue;

        if (auto fireOdds = getFireOdds(region, spreadPos); fireOdds > 0.0f) {
            auto difficulty         = std::to_underlying(region.mLevel.getDifficulty());
            auto difficultyModifier = static_cast<float>(difficulty >= 0 && difficulty <= 3 ? 40 + 7 * difficulty : 40);
            if (auto spreadChance =
                    ((fireOdds + difficultyModifier) / static_cast<float>(fireAge + 30)) * (isHumid ? 0.5f : 1.0f);
                spreadChance > 0.0f) {
                auto randomValue = static_cast<float>(static_cast<int>(random.mRandom->mObject._genRandInt32()))
                                 * 2.328306436538696e-10f;
                if (randomValue * static_cast<float>(100 * (spreadPos.y > pos.y + 1 ? spreadPos.y - pos.y : 1))
                    <= spreadChance) {
                    std::array<BlockPos, 5> spreadPositions = {
                        spreadPos,
                        spreadPos.add({-1, 0, 0}),
                        spreadPos.add({1, 0, 0}),
                        spreadPos.add({0, 0, -1}),
                        spreadPos.add({0, 0, 1})
                    };
                    if (!region.getDimension().mHasWeather || weather.mRainLevel <= 0.2f
                        || !std::ranges::any_of(spreadPositions, [&](BlockPos const& checkPos) {
                        return weather.isPrecipitatingAt(region, checkPos)
                            && region.getBiome(checkPos).getTemperature(region, checkPos) > 0.15000001f;
                    })) {
                        auto event = FireSpreadingEvent{region, pos, spreadPos};
                        getLLEventBus().publish(event);
                        if (!event.isCancelled()) {
                            region.setBlock(spreadPos, fireBlock, 3, 0, BlockChangeContext{false});
                            getLLEventBus().publish(FireSpreadedEvent{region, pos, spreadPos});
                        }
                    }
                }
            }
        }
    }
}

EventHook(SoulFireSpawningEvent, SoulFireSpawnedEvent, <FireEventHook1>);
EventHook(FireAgingEvent, FireAgedEvent, <FireEventHook1>);
EventHook(FireRemovingEvent, FireRemovedEvent, <FireEventHook1>);
EventHook(FireSpreadingEvent, FireSpreadedEvent, <FireEventHook1>);

} // namespace ila::block::inline fire