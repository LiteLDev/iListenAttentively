#include "ila/event/block/fire/FireEvent.h"
#include "ila/base/Gloabl.i.h"
#include "ila/event/block/fire/FireAgeEvent.h"
#include "ila/event/block/fire/FireBurnBlockEvent.h"
#include "ila/event/block/fire/FireEvictBeehiveEvent.h"
#include "ila/event/block/fire/FireIgniteTNTEvent.h"
#include "ila/event/block/fire/FireLightCampfireEvent.h"
#include "ila/event/block/fire/FireRemoveEvent.h"
#include "ila/event/block/fire/FireSpreadEvent.h"
#include "ila/event/block/fire/SoulFireSpawnEvent.h"
#include "ila/utils/EventUtils.i.h"
#include "ila/utils/GameRuleUtils.i.h"
#include "mc/deps/shared_types/legacy/Facing.h"
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
#include <mc/deps/nbt/CompoundTag.h>
#include <mc/util/Random.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/ChunkBlockPos.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/Weather.h>
#include <mc/world/level/biome/Biome.h>
#include <mc/world/level/block/BeehiveBlock.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/BlockChangeContext.h>
#include <mc/world/level/block/BlockProperty.h>
#include <mc/world/level/block/CampfireBlock.h>
#include <mc/world/level/block/FireBlock.h>
#include <mc/world/level/block/FlameOdds.h>
#include <mc/world/level/block/VanillaBlockTypeIds.h>
#include <mc/world/level/block/VanillaStates.h>
#include <mc/world/level/block/block_events/BlockQueuedTickEvent.h>
#include <mc/world/level/chunk/LevelChunk.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/level/levelgen/structure/BoundingBox.h>
#include <mc/world/level/material/Material.h>
#include <mc/world/level/storage/GameRule.h>
#include <mc/world/level/storage/GameRuleId.h>
#include <mc/world/level/storage/GameRules.h>
#include <mc/world/level/storage/LevelData.h>
#include <utility>

namespace ila::block::inline fire {

void FireEvent::serialize(CompoundTag& nbt) const {
    WorldEvent::serialize(nbt);
    reflection::serialize_to(nbt["pos"], mPos).value();
}

class FireInterceptCache {
public:
    struct State {
        ll::SmallDenseSet<BlockPos>                mBurntOutBlocksPrevented{};
        ll::SmallDenseSet<BlockPos>                mSpreadBlocksPrevented{};
        ll::SmallDenseSet<BlockPos>                mIgniteBlocksPrevented{};
        ll::SmallDenseSet<BlockPos>                mTNTBlocksPrevented{};
        ll::SmallDenseSet<BlockPos>                mEvictBeehiveBlocksPrevented{};
        ll::SmallDenseSet<FireRemoveEvent::Reason> mRemovalReasons{};
        bool                                       mAgingPrevented{false};
        bool                                       mSoulFirePrevented{false};
    };

private:
    ll::SmallDenseMap<DimensionType, ll::SmallDenseMap<ChunkPos, ll::SmallDenseMap<ChunkBlockPos, State>>> mCache;

public:
    static FireInterceptCache& getInstance() {
        static FireInterceptCache instance;
        return instance;
    }

    optional_ref<State> getCache(Dimension& dimension, BlockPos const& pos) {
        if (auto it = mCache.find(dimension.mId); it != mCache.end()) {
            if (auto it2 = it->second.find(ChunkPos{pos.x >> 4, pos.z >> 4}); it2 != it->second.end()) {
                if (auto it3 = it2->second.find(ChunkBlockPos{pos, dimension.mHeightRange->mMin});
                    it3 != it2->second.end()) {
                    return it3->second;
                }
            }
        }
        return std::nullopt;
    }

    State& getOrCreateCache(Dimension& dimension, BlockPos const& pos) {
        return mCache[dimension.mId][ChunkPos{pos.x >> 4, pos.z >> 4}]
                     [ChunkBlockPos{pos, dimension.mHeightRange->mMin}];
    }

    void clearForBlock(Dimension& dimension, BlockPos const& pos) {
        if (auto it = mCache.find(dimension.mId); it != mCache.end()) {
            if (auto it2 = it->second.find(ChunkPos{pos.x >> 4, pos.z >> 4}); it2 != it->second.end()) {
                it2->second.erase(ChunkBlockPos{pos, dimension.mHeightRange->mMin});
                if (it2->second.empty()) it->second.erase(it2);
            }
            if (it->second.empty()) mCache.erase(it);
        }
    }

    void clearForChunk(Dimension& dimension, ChunkPos const& chunkPos) {
        if (auto it = mCache.find(dimension.mId); it != mCache.end()) {
            it->second.erase(chunkPos);
            if (it->second.empty()) mCache.erase(it);
        }
    }
};

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

    if (FireInterceptCache::getInstance().getCache(region.mDimension, pos).and_then([](auto&& cache) {
        return cache.mSoulFirePrevented;
    })) {
        return false;
    }

    if (eventPromise(SoulFireSpawningEvent{region, pos})
            .onCancel([&] {
        FireInterceptCache::getInstance().getOrCreateCache(region.mDimension, pos).mSoulFirePrevented = true;
    }).publish()) {
        return false;
    }

    if (region.setBlock(pos, *soulFireBlock->mDefaultState, 3, nullptr, BlockChangeContext{})) {
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
    auto& region     = eventData.mRegion;
    auto& pos        = *eventData.mPos;
    auto& random     = eventData.mRandom;
    auto& gameRules  = *region.mLevel.mLevelData->get()->mGameRules;
    auto& weather    = *region.mDimension.mWeather;
    auto  fireBlock  = gsl::not_null<Block const*>{&region.getBlock(pos)};
    auto  belowPos   = pos.add({0, -1, 0});
    auto& belowBlock = region.getBlock(belowPos);
    auto  fireAge    = fireBlock->getState<int>(VanillaStates::Age()).value_or(0);
    auto  isHumid    = region.getBiome(pos).isHumid();
    auto  cache      = FireInterceptCache::getInstance().getCache(region.mDimension, pos);

    auto addToRandomTickingQueue = [&](BlockSource& region, BlockPos const& pos) {
        if (!region.isInstaticking(pos) && !region.hasTickInPendingTicks(pos)) {
            auto delay = random.nextInt(10) + 30;
            region.addToRandomTickingQueue(pos, *fireBlock->mBlockType->mDefaultState, delay, 0, 0);
        }
    };

    if (_trySpawnSoulFire(region, pos)) return;

    auto infiniBurn = [&]() {
        if (auto state = belowBlock.getState<bool>(VanillaStates::InfiniburnBit()); state) return *state;
        return static_cast<bool>(
            std::to_underlying(belowBlock.mBlockType->mProperties) & std::to_underlying(BlockProperty::InfiniBurn)
        );
    }();

    if (!mayPlace(region, pos) && !cache.and_then([](auto&& cache) {
        return cache.mRemovalReasons.contains(FireRemoveEvent::Reason::Unsupported);
    })) {
        if (!eventPromise(FireRemovingEvent{region, pos, FireRemoveEvent::Reason::Unsupported})
                 .onSuccess([&] { region.removeBlock(pos, BlockChangeContext{}); })
                 .onSuccessEvent(FireRemovedEvent{region, pos, FireRemoveEvent::Reason::Unsupported})
                 .onCancel([&] {
            FireInterceptCache::getInstance()
                .getOrCreateCache(region.mDimension, pos)
                .mRemovalReasons.insert(FireRemoveEvent::Reason::Unsupported);
        }).publish()) {
            return;
        }
    }

    if (!gamerule_utils::getGameRule(gameRules, GameRules::GameRulesIndex::DoFireTick, false)) {
        addToRandomTickingQueue(region, pos);
        return;
    }

    // clang-format off
    if (
        !gamerule_utils::getGameRule(gameRules, GameRules::GameRulesIndex::AllowDestructiveObjects, true)
        && !cache.and_then([](auto&& cache) {
        return cache.mRemovalReasons.contains(FireRemoveEvent::Reason::GameRule);
    })
        && !eventPromise(FireRemovingEvent{region, pos, FireRemoveEvent::Reason::GameRule})
                .onSuccess([&] { region.removeBlock(pos, BlockChangeContext{}); })
                .onSuccessEvent(FireRemovedEvent{region, pos, FireRemoveEvent::Reason::GameRule})
                .onCancel([&] {
        FireInterceptCache::getInstance()
            .getOrCreateCache(region.mDimension, pos)
            .mRemovalReasons.insert(FireRemoveEvent::Reason::GameRule);
            }).publish()
    ) {
        return;
    }

    if (!infiniBurn && weather.isRaining()) {
        // clang-format on
        auto rainingNearby = weather.isRainingAt(region, pos)
            || weather.isRainingAt(region, pos.add({1, 0, 0}))
            || weather.isRainingAt(region, pos.add({-1, 0, 0}))
            || weather.isRainingAt(region, pos.add({0, 0, -1}))
            || weather.isRainingAt(region, pos.add({0, 0, 1}));
        if (rainingNearby && !cache.and_then([](auto&& cache) {
            return cache.mRemovalReasons.contains(FireRemoveEvent::Reason::Rain);
        })) {
            if (!eventPromise(FireRemovingEvent{region, pos, FireRemoveEvent::Reason::Rain})
                     .onSuccess([&] { region.removeBlock(pos, BlockChangeContext{}); })
                     .onSuccessEvent(FireRemovedEvent{region, pos, FireRemoveEvent::Reason::Rain})
                     .onCancel([&] {
                FireInterceptCache::getInstance()
                    .getOrCreateCache(region.mDimension, pos)
                    .mRemovalReasons.insert(FireRemoveEvent::Reason::Rain);
            }).publish()) {
                return;
            }
        }
    }

    auto tryRemoveFire = [&](FireRemoveEvent::Reason reason) -> bool {
        if (cache.and_then([reason](auto&& cache) { return cache.mRemovalReasons.contains(reason); })) {
            return false;
        }
        return !eventPromise(FireRemovingEvent{region, pos, reason})
                    .onSuccess([&] { region.removeBlock(pos, BlockChangeContext{}); })
                    .onSuccessEvent(FireRemovedEvent{region, pos, reason})
                    .onCancel([&] {
            FireInterceptCache::getInstance().getOrCreateCache(region.mDimension, pos).mRemovalReasons.insert(reason);
        }).publish();
    };

    if (!infiniBurn && fireAge < 15 && !cache.and_then([](auto&& cache) { return cache.mAgingPrevented; })) {
        auto prevAge = fireAge;
        auto newAge  = fireAge + random.nextInt(3) / 2;
        if (newAge != fireAge) {
            if (auto event = eventPromise(FireAgingEvent{region, pos, fireAge, newAge})
                                 .onCancel([&] {
                FireInterceptCache::getInstance().getOrCreateCache(region.mDimension, pos).mAgingPrevented = true;
            }).publish();
                !event && event->newAge() != fireAge) {
                fireAge = event->newAge();
                if (auto newFireBlock = fireBlock->setState(VanillaStates::Age(), fireAge); newFireBlock) {
                    fireBlock = newFireBlock.as_ptr();
                }
            }
        }
        if (region.setBlock(pos, *fireBlock, 1, nullptr, BlockChangeContext{})) {
            eventPromise(FireAgedEvent{region, pos, prevAge, fireAge}).publish();
        }
    }

    addToRandomTickingQueue(region, pos);

    if (!infiniBurn) {
        auto belowMaterialType = belowBlock.mBlockType->mMaterial.mType;
        auto doTntExplode      = gamerule_utils::getGameRule(gameRules, GameRules::GameRulesIndex::DoTntExplode, false);

        if (belowMaterialType == SharedTypes::v1_26_20::MaterialType::Explosive && !doTntExplode) {
            if (fireAge < 4) {
                return;
            }
        }

        auto& liquidBelow = region.getLiquidBlock(belowPos);
        bool  isWater     = liquidBelow.mBlockType->mMaterial.mType == SharedTypes::v1_26_20::MaterialType::Water;
        if (!isValidFireLocation(region, pos) || isWater) {
            if (fireAge <= 3 && canProvideFullSupport(liquidBelow, static_cast<uchar>(SharedTypes::Facing::Up))) {
                return;
            } else {
                tryRemoveFire(isWater ? FireRemoveEvent::Reason::Water : FireRemoveEvent::Reason::Unsupported);
                return;
            }
        }

        if (fireAge == 15 && belowBlock.mDirectData->mFlameOdds == FlameOdds::Never && !random.nextInt(4)) {
            tryRemoveFire(FireRemoveEvent::Reason::BurntOut);
            return;
        }
    }

    checkBurn(region, pos.add({1, 0, 0}), isHumid ? 250 : 300, random, fireAge, pos);
    checkBurn(region, pos.add({-1, 0, 0}), isHumid ? 250 : 300, random, fireAge, pos);
    checkBurn(region, pos.add({0, -1, 0}), isHumid ? 200 : 250, random, fireAge, pos);
    checkBurn(region, pos.add({0, 1, 0}), isHumid ? 200 : 250, random, fireAge, pos);
    checkBurn(region, pos.add({0, 0, -1}), isHumid ? 250 : 300, random, fireAge, pos);
    checkBurn(region, pos.add({0, 0, 1}), isHumid ? 250 : 300, random, fireAge, pos);

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
                    auto rainingNearby = weather.isRainingAt(region, spreadPos)
                                      || weather.isRainingAt(region, spreadPos.add({-1, 0, 0}))
                                      || weather.isRainingAt(region, spreadPos.add({1, 0, 0}))
                                      || weather.isRainingAt(region, spreadPos.add({0, 0, -1}))
                                      || weather.isRainingAt(region, spreadPos.add({0, 0, 1}));
                    if (!weather.isRaining() || !rainingNearby) {
                        if (!cache.and_then([&](auto&& cache) {
                            return cache.mSpreadBlocksPrevented.contains(spreadPos);
                        })) {
                            eventPromise(FireSpreadingEvent{region, pos, spreadPos})
                                .onSuccess([&] {
                                region.setBlock(spreadPos, *fireBlock, 3, nullptr, BlockChangeContext{});
                            })
                                .onSuccessEvent(FireSpreadedEvent{region, pos, spreadPos})
                                .onCancel([&] {
                                FireInterceptCache::getInstance()
                                    .getOrCreateCache(region.mDimension, pos)
                                    .mSpreadBlocksPrevented.insert(spreadPos);
                            }).publish();
                        }
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
    IRandom&        random,
    int             age,
    BlockPos const& firePos
) {
    auto nextInt = [&random](int max) { return random.nextInt(max + 1); };

    auto& block     = region.getBlock(pos);
    auto& blockType = *block.mBlockType;
    auto  cache     = FireInterceptCache::getInstance().getCache(region.mDimension, firePos);

    if (*blockType.mNameInfo->mFullName == VanillaBlockTypeIds::Beehive()
        || *blockType.mNameInfo->mFullName == VanillaBlockTypeIds::BeeNest()) {
        if (!cache.and_then([&](auto&& cache) { return cache.mEvictBeehiveBlocksPrevented.contains(pos); })) {
            eventPromise(FireEvictingBeehiveEvent{region, firePos, pos})
                .onSuccess([&] { static_cast<BeehiveBlock&>(blockType).evictAll(region, pos, false); })
                .onSuccessEvent(FireEvictedBeehiveEvent{region, firePos, pos})
                .onCancel([&] {
                FireInterceptCache::getInstance()
                    .getOrCreateCache(region.mDimension, firePos)
                    .mEvictBeehiveBlocksPrevented.insert(pos);
            }).publish();
        }
    }

    if (nextInt(chance - 1) < std::to_underlying(block.mDirectData->mBurnOdds)) {
        bool isTnt      = (*blockType.mNameInfo->mFullName == VanillaBlockTypeIds::Tnt());
        bool isCampfire = (*blockType.mNameInfo->mFullName == VanillaBlockTypeIds::CampFire())
                       || (*blockType.mNameInfo->mFullName == VanillaBlockTypeIds::SoulCampfire());

        if (nextInt(age + 9) >= 5 || region.mDimension.mWeather->isRainingAt(region, pos)) {
            if (!isCampfire && !cache.and_then([&](auto&& cache) {
                return cache.mBurntOutBlocksPrevented.contains(pos);
            }) && !eventPromise(FireBurningBlockEvent{region, firePos, pos}).publish()) {
                if (isTnt) {
                    if (region.mLevel.mLevelData->get()->mGameRules->getBool(
                            GameRuleId{std::to_underlying(GameRules::GameRulesIndex::DoNaturalRegeneration)},
                            false
                        )) {
                        region.removeBlock(pos, BlockChangeContext{});
                        eventPromise(FireBurnedBlockEvent{region, firePos, pos}).publish();
                        FireInterceptCache::getInstance()
                            .getOrCreateCache(region.mDimension, firePos)
                            .mBurntOutBlocksPrevented.insert(pos);
                        return;
                    }
                } else {
                    region.removeBlock(pos, BlockChangeContext{});
                    eventPromise(FireBurnedBlockEvent{region, firePos, pos}).publish();
                    FireInterceptCache::getInstance()
                        .getOrCreateCache(region.mDimension, firePos)
                        .mBurntOutBlocksPrevented.insert(pos);
                }
            }
        } else if (!isCampfire && !isTnt) {
            auto fireBlockWithAge =
                mDefaultState->setState<bool>(VanillaStates::Age(), std::min(age + nextInt(4) / 4, 15));
            if (!cache.and_then([&](auto&& cache) { return cache.mBurntOutBlocksPrevented.contains(pos); })) {
                eventPromise(FireBurningBlockEvent{region, firePos, pos})
                    .onSuccess([&] { region.removeBlock(pos, BlockChangeContext{}); })
                    .onSuccessEvent(FireBurnedBlockEvent{region, firePos, pos})
                    .onCancel([&] {
                    FireInterceptCache::getInstance()
                        .getOrCreateCache(region.mDimension, firePos)
                        .mBurntOutBlocksPrevented.insert(pos);
                }).publish();
            }
            if (isValidFireLocation(region, pos) && fireBlockWithAge
                && !cache.and_then([&](auto&& cache) { return cache.mSpreadBlocksPrevented.contains(pos); })) {
                auto spreadPos = pos;
                eventPromise(FireSpreadingEvent{region, firePos, spreadPos})
                    .onSuccess([&] { region.setBlock(spreadPos, *fireBlockWithAge, 3, nullptr, BlockChangeContext{}); })
                    .onSuccessEvent(FireSpreadedEvent{region, firePos, spreadPos})
                    .onCancel([&] {
                    FireInterceptCache::getInstance()
                        .getOrCreateCache(region.mDimension, firePos)
                        .mSpreadBlocksPrevented.insert(pos);
                }).publish();
            }
            return;
        }

        if (isCampfire && !cache.and_then([&](auto&& cache) { return cache.mIgniteBlocksPrevented.contains(pos); })) {
            eventPromise(FireLightingCampfireEvent{region, firePos, pos})
                .onSuccess([&] {
                if (CampfireBlock::tryLightFire(region, pos, nullptr)
                    && region.getBlock(pos).getState<bool>(VanillaStates::Extinguished())) {
                    eventPromise(FireLightedCampfireEvent{region, firePos, pos}).publish();
                }
            })
                .onCancel([&] {
                FireInterceptCache::getInstance()
                    .getOrCreateCache(region.mDimension, firePos)
                    .mIgniteBlocksPrevented.insert(pos);
            }).publish();
        }

        if (isTnt && !cache.and_then([&](auto&& cache) { return cache.mTNTBlocksPrevented.contains(pos); })) {
            eventPromise(FireIgnitingTNTEvent{region, firePos, pos})
                .onSuccess([&] {
                auto tntWithExplode = block.setState<int>(VanillaStates::ExplodeBit(), true);
                tntWithExplode.value_or(block).mBlockType->destroy(region, pos, block, nullptr);

                if (region.mLevel.mLevelData->get()->mGameRules->getBool(
                        GameRuleId{std::to_underlying(GameRules::GameRulesIndex::DoNaturalRegeneration)},
                        false
                    )) {
                    region.removeBlock(pos, BlockChangeContext{});
                }
            })
                .onSuccessEvent(FireIgnitedTNTEvent{region, firePos, pos})
                .onCancel([&] {
                FireInterceptCache::getInstance()
                    .getOrCreateCache(region.mDimension, firePos)
                    .mTNTBlocksPrevented.insert(pos);
            }).publish();
        }
    }
}

LL_TYPE_INSTANCE_HOOK(FireEventHook4, HookPriority::Normal, Level, &Level::_onChunkDiscarded, void, LevelChunk& lc) {
    origin(lc);
    FireInterceptCache::getInstance().clearForChunk(lc.mDimension, lc.mPosition);
}

LL_TYPE_INSTANCE_HOOK(
    FireEventHook5,
    HookPriority::Normal,
    FireBlock,
    &FireBlock::$neighborChanged,
    void,
    BlockSource&    region,
    BlockPos const& pos,
    BlockPos const& neighborPos
) {
    origin(region, pos, neighborPos);
    FireInterceptCache::getInstance().clearForBlock(region.mDimension, pos);
}

LL_TYPE_INSTANCE_HOOK(
    FireEventHook6,
    HookPriority::Normal,
    LevelChunk,
    &LevelChunk::_removeCallbacks,
    void,
    ChunkBlockPos const&      pos,
    Block const&              oldBlock,
    Block const&              current,
    BlockSource*              currentSource,
    BlockChangeContext const& changeSourceContext
) {
    origin(pos, oldBlock, current, currentSource, changeSourceContext);
    static auto vsFireType = VanillaBlockTypeIds::Fire();
    if (currentSource && *oldBlock.mBlockType->mNameInfo->mFullName == vsFireType
        && *oldBlock.mBlockType->mNameInfo->mFullName != *current.mBlockType->mNameInfo->mFullName) {
        FireInterceptCache::getInstance().clearForBlock(
            currentSource->mDimension,
            BlockPos{
                pos.x + mPosition->x * 16,
                pos.y.mVal + currentSource->mDimension.mHeightRange->mMin,
                pos.z + mPosition->z * 16
            }
        );
    }
}

#define CacheHook FireEventHook4, FireEventHook5, FireEventHook6
EventHook(SoulFireSpawningEvent, SoulFireSpawnedEvent, <FireEventHook1, CacheHook>);
EventHook(FireAgingEvent, FireAgedEvent, <FireEventHook2, CacheHook>);
EventHook(FireRemovingEvent, FireRemovedEvent, <FireEventHook2, CacheHook>);
EventHook(FireSpreadingEvent, FireSpreadedEvent, <FireEventHook2, FireEventHook3, CacheHook>);
EventHook(FireEvictingBeehiveEvent, FireEvictedBeehiveEvent, <FireEventHook3, CacheHook>);
EventHook(FireBurningBlockEvent, FireBurnedBlockEvent, <FireEventHook3, CacheHook>);
EventHook(FireLightingCampfireEvent, FireLightedCampfireEvent, <FireEventHook3, CacheHook>);
EventHook(FireIgnitingTNTEvent, FireIgnitedTNTEvent, <FireEventHook3, CacheHook>);
#undef CacheHook

} // namespace ila::block::inline fire