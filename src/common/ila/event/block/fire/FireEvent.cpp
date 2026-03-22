#include "ila/event/block/fire/FireEvent.h"
#include "ila/base/Gloabl.i.h"
#include "ila/event/block/fire/FireAgeEvent.h"
#include "ila/event/block/fire/FireRemoveEvent.h"
#include "ila/event/block/fire/FireSpreadEvent.h"
#include "ila/event/block/fire/FireBurnBlockEvent.h"
#include "ila/event/block/fire/SoulFireSpawnEvent.h"
#include "ila/event/block/fire/FireLightCampfireEvent.h"
#include "ila/event/block/fire/FireEvictBeehiveEvent.h"
#include "ila/event/block/fire/FireIgniteTNTEvent.h"
#include "ila/utils/EventUtils.i.h"
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
#include <mc/world/level/levelgen/structure/BoundingBox.h>
#include <mc/world/level/Weather.h>
#include <mc/world/level/biome/Biome.h>
#include <mc/world/level/block/BeehiveBlock.h>
#include <mc/world/level/block/CampfireBlock.h>
#include <mc/world/level/block/VanillaBlockTypeIds.h>
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
    &FireBlock::_trySpawnSoulFire,
    bool,
    BlockSource&    region,
    BlockPos const& pos
) {
    auto& belowBlock = *region.getBlock(BlockPos{pos.x, pos.y - 1, pos.z}).mBlockType;
    if (*belowBlock.mNameInfo->mFullName != VanillaBlockTypeIds::SoulSoil()
        && *belowBlock.mNameInfo->mFullName != VanillaBlockTypeIds::SoulSand()) {
        return false;
    }

    auto soulFireBlock = BlockType::tryGetFromRegistry(VanillaBlockTypeIds::SoulFire());
    if (!soulFireBlock) return false;

    if (!soulFireBlock->mMinRequiredBaseGameVersion->isCompatibleWith(
            region.mLevel.mLevelData->get()->getBaseGameVersion()
        )) {
        return false;
    }

    if (eventPromise(SoulFireSpawningEvent{region, pos}).publish()) return false;

    if (region.setBlock(pos, *soulFireBlock->mDefaultState, 3, nullptr, BlockChangeContext{false})) {
        eventPromise(SoulFireSpawnedEvent{region, pos}).publish();
        return true;
    }
    return false;
}

LL_TYPE_INSTANCE_HOOK(
    FireEventHook2,
    HookPriority::Low,
    FireBlock,
    &FireBlock::tick,
    void,
    BlockEvents::BlockQueuedTickEvent& eventData
) {
    auto&     region     = eventData.mRegion;
    auto&     pos        = *eventData.mPos;
    auto&     random     = eventData.mRandom;
    auto&     gameRules  = *region.mLevel.mLevelData->get()->mGameRules;
    auto&     weather    = *region.mDimension.mWeather;
    auto      fireBlock  = gsl::not_null<Block const*>{&region.getBlock(pos)};
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

    if (_trySpawnSoulFire(region, pos)) return;

    auto infiniBurn = [&]() {
        if (auto state = belowBlock.getState<bool>(VanillaStates::InfiniburnBit()); state) return *state;
        return static_cast<bool>(
            std::to_underlying(belowBlock.mBlockType->mProperties) & std::to_underlying(BlockProperty::InfiniBurn)
        );
    }();

    if (!mayPlace(region, pos)) {
        if (!eventPromise(FireRemovingEvent{region, pos, FireRemoveEvent::Reason::Unsupported})
                 .onSuccess([&] { region.removeBlock(pos, BlockChangeContext{false}); })
                 .onSuccessEvent(FireRemovedEvent{region, pos, FireRemoveEvent::Reason::Unsupported})
                 .publish()) {
            return;
        }
    }

    if (!getGameRule(GameRules::GameRulesIndex::DoFireTick, false)) {
        _tryAddToTickingQueue(region, pos, random);
        return;
    }

    if (!getGameRule(GameRules::GameRulesIndex::AllowDestructiveObjects, true)
        && !eventPromise(FireRemovingEvent{region, pos, FireRemoveEvent::Reason::GameRule})
                .onSuccess([&] { region.removeBlock(pos, BlockChangeContext{false}); })
                .onSuccessEvent(FireRemovedEvent{region, pos, FireRemoveEvent::Reason::GameRule})
                .publish()) {
        return;
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
        if (!eventPromise(FireRemovingEvent{region, pos, FireRemoveEvent::Reason::Rain})
                 .onSuccess([&] { region.removeBlock(pos, BlockChangeContext{false}); })
                 .onSuccessEvent(FireRemovedEvent{region, pos, FireRemoveEvent::Reason::Rain})
                 .publish()) {
            return;
        }
    }

    auto tryRemoveFire = [&](FireRemoveEvent::Reason reason) -> bool {
        return !eventPromise(FireRemovingEvent{region, pos, reason})
                    .onSuccess([&] { region.removeBlock(pos, BlockChangeContext{false}); })
                    .onSuccessEvent(FireRemovedEvent{region, pos, reason})
                    .publish();
    };

    if (![&]() { // 返回值代表是否蔓延火焰
        if (infiniBurn) return true;

        if (fireAge < 15) {
            auto prevAge = fireAge;

            if (auto newAge = fireAge + random.nextInt(3) / 2; newAge != fireAge) {
                if (auto event = eventPromise(FireAgingEvent{region, pos, fireAge, newAge}).publish();
                    !event && event->newAge() != fireAge) {
                    fireAge = event->newAge();
                    if (auto newFireBlock = fireBlock->setState(VanillaStates::Age(), fireAge); newFireBlock) {
                        fireBlock = newFireBlock.as_ptr();
                    }
                }
            }

            if (region.setBlock(pos, *fireBlock, 1, nullptr, BlockChangeContext{false})) {
                eventPromise(FireAgedEvent{region, pos, prevAge, fireAge}).publish();
            }
        }

        auto belowMaterialType = belowBlock.mBlockType->mMaterial.mType;
        if (belowMaterialType == MaterialType::Explosive) {
            bool doTntExplode = getGameRule(GameRules::GameRulesIndex::DoTntExplode, false);
            if (!doTntExplode) {
                if (fireAge > 3) {
                    tryRemoveFire(FireRemoveEvent::Reason::GameRule);
                    return false;
                }
                return false;
            }
        }

        if (!isValidFireLocation(region, pos)) {
            if (!isSolidToppedBlock(region, belowPos)) {
                tryRemoveFire(FireRemoveEvent::Reason::Unsupported);
                return false;
            }
        }

        auto& liquidBelow = region.getLiquidBlock(belowPos);
        if (liquidBelow.mBlockType->mMaterial.mType == MaterialType::Water) {
            if (!isSolidToppedBlock(region, belowPos) && fireAge > 3) {
                tryRemoveFire(FireRemoveEvent::Reason::Water);
            }
            return false;
        }

        if (belowBlock.mDirectData->mFlameOdds == FlameOdds::Never && fireAge == 15 && !random.nextInt(4)) {
            tryRemoveFire(FireRemoveEvent::Reason::BurntOut);
            return false;
        }

        return true;
    }()) {
        _tryAddToTickingQueue(region, pos, random);
        return;
    }
    _tryAddToTickingQueue(region, pos, random);

    checkBurn(region, pos.add({1, 0, 0}), isHumid ? 250 : 300, randomize, fireAge, pos);
    checkBurn(region, pos.add({-1, 0, 0}), isHumid ? 250 : 300, randomize, fireAge, pos);
    checkBurn(region, pos.add({0, -1, 0}), isHumid ? 200 : 250, randomize, fireAge, pos);
    checkBurn(region, pos.add({0, 1, 0}), isHumid ? 200 : 250, randomize, fireAge, pos);
    checkBurn(region, pos.add({0, 0, -1}), isHumid ? 250 : 300, randomize, fireAge, pos);
    checkBurn(region, pos.add({0, 0, 1}), isHumid ? 250 : 300, randomize, fireAge, pos);

    // clang-format off
    for (auto dPos : BoundingBox{-1, {1, 4, 1}}.forEachPos()) {
        // clang-format on

        auto spreadPos = pos.add(dPos);
        // if (region.getBlock(spreadPos).isAir()) continue; // 用于拦截火焰烧毁方块使用 (原版在checkBurn逻辑中把方块烧毁的)

        if (auto fireOdds = getFireOdds(region, spreadPos); fireOdds > 0.0f) {
            auto difficulty = std::to_underlying(region.mLevel.getDifficulty());
            auto difficultyModifier =
                static_cast<float>(difficulty >= 0 && difficulty <= 3 ? (40 + 7 * difficulty) : 40);
            if (auto spreadChance =
                    (((fireOdds + difficultyModifier) / static_cast<float>(fireAge + 30))) * (isHumid ? 0.5f : 1.0f);
                spreadChance > 0.0f) {
                auto randomValue = static_cast<float>(static_cast<uint32_t>(random.mRandom->mObject._genRandInt32()))
                                 * 2.328306436538696e-10f;
                auto spreadRate = dPos.y > 1 ? 100 * dPos.y : 100;
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
                        eventPromise(FireSpreadingEvent{region, pos, spreadPos})
                            .onSuccess([&] {
                            region.setBlock(spreadPos, *fireBlock, 3, nullptr, BlockChangeContext{false});
                        })
                            .onSuccessEvent(FireSpreadedEvent{region, pos, spreadPos})
                            .publish();
                    }
                }
            }
        }
    }
}

LL_TYPE_INSTANCE_HOOK(
    FireEventHook3,
    HookPriority::Low,
    FireBlock,
    &FireBlock::checkBurn,
    void,
    BlockSource&    region,
    BlockPos const& pos,
    int             chance,
    Randomize&      randomize,
    int             age,
    BlockPos const& firePos
) {
    auto nextInt = [&randomize](int max) { return randomize.mRandom->mPointer->nextInt(max + 1); };

    auto& block     = region.getBlock(pos);
    auto& blockType = *block.mBlockType;

    if (*blockType.mNameInfo->mFullName == VanillaBlockTypeIds::Beehive()
        || *blockType.mNameInfo->mFullName == VanillaBlockTypeIds::BeeNest()) {
        eventPromise(FireEvictingBeehiveEvent{region, firePos, pos})
            .onSuccess([&] { static_cast<BeehiveBlock&>(blockType).evictAll(region, pos, false); })
            .onSuccessEvent(FireEvictedBeehiveEvent{region, firePos, pos})
            .publish();
    }

    if (nextInt(chance - 1) < std::to_underlying(block.mDirectData->mBurnOdds)) {
        bool isTnt      = (*blockType.mNameInfo->mFullName == VanillaBlockTypeIds::Tnt());
        bool isCampfire = (*blockType.mNameInfo->mFullName == VanillaBlockTypeIds::CampFire())
                       || (*blockType.mNameInfo->mFullName == VanillaBlockTypeIds::SoulCampfire());

        if (nextInt(age + 9) >= 5 || region.mDimension.mWeather->isRainingAt(region, pos)) {
            if (!isCampfire && !eventPromise(FireBurningBlockEvent{region, firePos, pos}).publish()) {
                if (isTnt) {
                    if (region.mLevel.mLevelData->get()->mGameRules->getBool(
                            GameRuleId{std::to_underlying(GameRules::GameRulesIndex::DoNaturalRegeneration)},
                            false
                        )) {
                        region.removeBlock(pos, BlockChangeContext{false});
                        eventPromise(FireBurnedBlockEvent{region, firePos, pos}).publish();
                        return;
                    }
                } else {
                    region.removeBlock(pos, BlockChangeContext{false});
                    eventPromise(FireBurnedBlockEvent{region, firePos, pos}).publish();
                }
            }
        } else {
            auto fireBlockWithAge =
                mDefaultState->setState<bool>(VanillaStates::Age(), std::min(age + nextInt(4) / 4, 15));

            if (!isCampfire) {
                if (!isTnt) {
                    eventPromise(FireBurningBlockEvent{region, firePos, pos})
                        .onSuccess([&] { region.removeBlock(pos, BlockChangeContext{false}); })
                        .onSuccessEvent(FireBurnedBlockEvent{region, firePos, pos})
                        .publish();
                    if (isValidFireLocation(region, pos) && fireBlockWithAge) {
                        auto spreadPos = pos;
                        eventPromise(FireSpreadingEvent{region, firePos, spreadPos})
                            .onSuccess([&] {
                            region.setBlock(spreadPos, *fireBlockWithAge, 3, nullptr, BlockChangeContext{false});
                        })
                            .onSuccessEvent(FireSpreadedEvent{region, firePos, spreadPos})
                            .publish();
                    }
                    return;
                }
            }
        }

        if (isCampfire) {
            eventPromise(FireLightingCampfireEvent{region, firePos, pos})
                .onSuccess([&] {
                if (CampfireBlock::tryLightFire(region, pos, nullptr)
                    && region.getBlock(pos).getState<bool>(VanillaStates::Extinguished())) {
                    eventPromise(FireLightedCampfireEvent{region, firePos, pos}).publish();
                }
            }).publish();
        }

        if (isTnt) {
            eventPromise(FireIgnitingTNTEvent{region, firePos, pos})
                .onSuccess([&] {
                auto tntWithExplode = block.setState<int>(VanillaStates::ExplodeBit(), true);
                tntWithExplode.value_or(block).mBlockType->destroy(region, pos, block, nullptr);

                if (region.mLevel.mLevelData->get()->mGameRules->getBool(
                        GameRuleId{std::to_underlying(GameRules::GameRulesIndex::DoNaturalRegeneration)},
                        false
                    )) {
                    region.removeBlock(pos, BlockChangeContext{false});
                }
            })
                .onSuccessEvent(FireIgnitedTNTEvent{region, firePos, pos})
                .publish();
        }
    }
}

EventHook(SoulFireSpawningEvent, SoulFireSpawnedEvent, <FireEventHook1>);
EventHook(FireAgingEvent, FireAgedEvent, <FireEventHook2>);
EventHook(FireRemovingEvent, FireRemovedEvent, <FireEventHook2>);
EventHook(FireSpreadingEvent, FireSpreadedEvent, <FireEventHook2, FireEventHook3>);
EventHook(FireEvictingBeehiveEvent, FireEvictedBeehiveEvent, <FireEventHook3>);
EventHook(FireBurningBlockEvent, FireBurnedBlockEvent, <FireEventHook3>);
EventHook(FireLightingCampfireEvent, FireLightedCampfireEvent, <FireEventHook3>);
EventHook(FireIgnitingTNTEvent, FireIgnitedTNTEvent, <FireEventHook3>);

} // namespace ila::block::inline fire
