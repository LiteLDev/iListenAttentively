#include "ila/event/block/fire/FireEvent.h"
#include "ila/base/Gloabl.i.h"
#include "ila/event/block/fire/FireAgeEvent.h"
#include "ila/event/block/fire/FireRemoveEvent.h"
#include "ila/event/block/fire/FireSpreadEvent.h"
#include "ila/event/block/fire/SoulFireSpawnEvent.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <ll/api/base/TypeTraits.h>
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
#include <mc/world/level/material/Material.h>
#include <mc/world/level/material/MaterialType.h>
#include <mc/world/level/storage/GameRule.h>
#include <mc/world/level/storage/GameRuleId.h>
#include <mc/world/level/storage/GameRules.h>
#include <mc/world/level/storage/LevelData.h>
#include <utility>
#include <variant>

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
    auto&     eventBus   = getLLEventBus();
    auto&     region     = eventData.mRegion;
    auto&     pos        = *eventData.mPos;
    auto&     random     = eventData.mRandom;
    auto&     gameRules  = *region.mLevel.mLevelData->get()->mGameRules;
    auto&     weather    = *region.mDimension.mWeather;
    auto*     fireBlock  = &region.getBlock(pos);
    auto      belowPos   = pos.add({0, -1, 0});
    auto&     belowBlock = region.getBlock(belowPos);
    auto      fireAge    = fireBlock->getState<int>(VanillaStates::Age()).value_or(0);
    auto      isHumid    = region.getBiome(pos).isHumid();
    Randomize randomize(random);

    auto getGameRule = [&gameRules]<typename T>
        requires(ll::traits::is_in_types_v<T, GameRule::Value>)
    (GameRules::GameRulesIndex id, T defaultValue) {
        auto index = static_cast<size_t>(std::to_underlying(id));
        if (gameRules.mGameRules->size() <= index) return defaultValue;
        auto& gameRule = (*gameRules.mGameRules)[index];
        if (auto result = std::get_if<T>(&*gameRule.mValue); result) return *result;
        return defaultValue;
    };

    {
        auto event = SoulFireSpawningEvent{region, pos};
        eventBus.publish(event);
        if (!event.isCancelled() && _trySpawnSoulFire(region, pos)) {
            eventBus.publish(SoulFireSpawnedEvent{region, pos});
            return;
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
        eventBus.publish(event);
        if (!event.isCancelled()) {
            region.removeBlock(pos, BlockChangeContext{false});
            eventBus.publish(FireRemovedEvent{region, pos, FireRemoveEvent::Reason::InvalidPos});
            return;
        }
    }

    if (!getGameRule(GameRules::GameRulesIndex::DoFireTick, false)) {
        _tryAddToTickingQueue(region, pos, random);
        return;
    }

    if (!getGameRule(GameRules::GameRulesIndex::AllowDestructiveObjects, true)) {
        auto event = FireRemovingEvent{region, pos, FireRemoveEvent::Reason::GameRule};
        eventBus.publish(event);
        if (!event.isCancelled()) {
            region.removeBlock(pos, BlockChangeContext{false});
            eventBus.publish(FireRemovedEvent{region, pos, FireRemoveEvent::Reason::GameRule});
            return;
        }
    }

    static std::array<std::function<bool(BlockSource&, Weather&, BlockPos const&)>, 5> vscNearbyRainfallCheck =
        {[](BlockSource& region, Weather& weather, BlockPos const& pos) {
        return weather.isPrecipitatingAt(region, pos) && region.getBiome(pos).getTemperature(region, pos) > 0.15000001f;
    }, [](BlockSource& region, Weather& weather, BlockPos const& pos) {
        auto nearby = pos.add({1, 0, 0});
        return weather.isPrecipitatingAt(region, nearby)
            && region.getBiome(nearby).getTemperature(region, nearby) > 0.15000001f;
    }, [](BlockSource& region, Weather& weather, BlockPos const& pos) {
        return weather.isRainingAt(region, pos.add({-1, 0, 0}));
    }, [](BlockSource& region, Weather& weather, BlockPos const& pos) {
        return weather.isRainingAt(region, pos.add({0, 0, -1}));
    }, [](BlockSource& region, Weather& weather, BlockPos const& pos) {
        return weather.isRainingAt(region, pos.add({0, 0, 1}));
    }};

    if (!infiniBurn && region.mDimension.mHasWeather && weather.mRainLevel > 0.2f
        && std::ranges::any_of(vscNearbyRainfallCheck, [&](auto&& check) { return check(region, weather, pos); })) {
        auto event = FireRemovingEvent{region, pos, FireRemoveEvent::Reason::Rain};
        eventBus.publish(event);
        if (!event.isCancelled()) {
            region.removeBlock(pos, BlockChangeContext{false});
            eventBus.publish(FireRemovedEvent{region, pos, FireRemoveEvent::Reason::Rain});
            return;
        }
    }

    auto tryRemoveFire = [&](FireRemoveEvent::Reason reason) -> bool {
        auto event = FireRemovingEvent{region, pos, reason};
        eventBus.publish(event);
        if (event.isCancelled()) return false;
        region.removeBlock(pos, BlockChangeContext{false});
        eventBus.publish(FireRemovedEvent{region, pos, reason});
        return true;
    };

    if (infiniBurn) {
        _tryAddToTickingQueue(region, pos, random);
    } else {
        if (fireAge < 15) {
            auto newAge = fireAge + random.nextInt(3) / 2;

            auto event = FireAgingEvent{region, pos, fireAge, newAge};
            eventBus.publish(event);
            if (!event.isCancelled() && event.newAge() != fireAge) {
                auto prevAge = fireAge;
                fireAge      = event.newAge();

                auto& block = fireBlock->mBlockType->trySetState<int>(VanillaStates::Age(), fireAge, fireBlock->mData)
                                  .value_or<Block const>(*fireBlock);
                region.setBlock(pos, block, 1, nullptr, BlockChangeContext{false});
                fireBlock = &region.getBlock(pos);
                eventBus.publish(FireAgedEvent{region, pos, prevAge, fireAge});
            }
        }

        _tryAddToTickingQueue(region, pos, random);

        auto belowMaterialType = belowBlock.mBlockType->mMaterial.mType;
        if (belowMaterialType == MaterialType::Explosive
            && getGameRule(GameRules::GameRulesIndex::DoTntExplode, false)) {
            return;
        }

        if (!isValidFireLocation(region, pos)) {
            tryRemoveFire(FireRemoveEvent::Reason::Naturally);
            return;
        }

        auto& liquidBelow = region.getLiquidBlock(belowPos);
        if (liquidBelow.mBlockType->mMaterial.mType == MaterialType::Water) {
            if (!isSolidToppedBlock(region, belowPos)) {
                tryRemoveFire(FireRemoveEvent::Reason::Naturally);
                return;
            }
            if (fireAge > 3) {
                tryRemoveFire(FireRemoveEvent::Reason::Naturally);
            }
            return;
        }

        if (belowBlock.mDirectData->mFlameOdds == FlameOdds::Never && fireAge == 15 && !random.nextInt(4)) {
            tryRemoveFire(FireRemoveEvent::Reason::Naturally);
            return;
        }
    }

    checkBurn(region, pos.add({1, 0, 0}), isHumid ? 250 : 300, randomize, fireAge, pos);
    checkBurn(region, pos.add({-1, 0, 0}), isHumid ? 250 : 300, randomize, fireAge, pos);
    checkBurn(region, pos.add({0, -1, 0}), isHumid ? 200 : 250, randomize, fireAge, pos);
    checkBurn(region, pos.add({0, 1, 0}), isHumid ? 200 : 250, randomize, fireAge, pos);
    checkBurn(region, pos.add({0, 0, -1}), isHumid ? 250 : 300, randomize, fireAge, pos);
    checkBurn(region, pos.add({0, 0, 1}), isHumid ? 250 : 300, randomize, fireAge, pos);

    for (int xx = -1; xx <= 1; ++xx) {
        for (int zz = -1; zz <= 1; ++zz) {
            for (int yy = -1; yy <= 4; ++yy) {
                if (xx == 0 && yy == 0 && zz == 0) continue;

                BlockPos spreadPos{pos.x + xx, pos.y + yy, pos.z + zz};

                if (auto fireOdds = getFireOdds(region, spreadPos); fireOdds > 0.0f) {
                    auto difficulty = std::to_underlying(region.mLevel.getDifficulty());
                    auto difficultyModifier =
                        static_cast<float>(difficulty >= 0 && difficulty <= 3 ? (40 + 7 * difficulty) : 40);
                    if (auto spreadChance = (((fireOdds + difficultyModifier) / static_cast<float>(fireAge + 30)))
                                          * (isHumid ? 0.5f : 1.0f);
                        spreadChance > 0.0f) {
                        auto randomValue =
                            static_cast<float>(static_cast<uint32_t>(random.mRandom->mObject._genRandInt32()))
                            * 2.328306436538696e-10f;
                        auto spreadRate = yy > 1 ? 100 * yy : 100;
                        if (randomValue * static_cast<float>(spreadRate) <= spreadChance) {
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
                                eventBus.publish(event);
                                if (!event.isCancelled()) {
                                    region.setBlock(spreadPos, *fireBlock, 3, nullptr, BlockChangeContext{false});
                                    eventBus.publish(FireSpreadedEvent{region, pos, spreadPos});
                                }
                            }
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
