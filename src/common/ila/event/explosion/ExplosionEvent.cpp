// clang-format off
#include "ila/utils/EventUtils.i.h"
#pragma include_alias("mc/world/level/block/ResourceDropsContext.h", "patch_mc/world/level/block/ResourceDropsContext.i.h")
#pragma include_alias(<mc/world/level/block/ResourceDropsContext.h>, <patch_mc/world/level/block/ResourceDropsContext.i.h>)
#pragma include_alias("mc/world/level/ParticlesBlockExplosionEvent.h", "patch_mc/world/level/ParticlesBlockExplosionEvent.i.h")
#pragma include_alias(<mc/world/level/ParticlesBlockExplosionEvent.h>, <patch_mc/world/level/ParticlesBlockExplosionEvent.i.h>)
#pragma include_alias("mc/network/packet/LevelEventGenericPacketPayload.h", "patch_mc/network/packet/LevelEventGenericPacketPayload.i.h")
#pragma include_alias(<mc/network/packet/LevelEventGenericPacketPayload.h>, <patch_mc/network/packet/LevelEventGenericPacketPayload.i.h>)
// clang-format on
#include "ila/event/explosion/ExplosionEvent.h"
#include "ila/base/Gloabl.i.h"
#include "ila/event/explosion/ExplosionCollisionOffsetEvent.h"
#include "ila/event/explosion/ExplosionDamageEntityEvent.h"
#include "ila/event/explosion/ExplosionDestroyBlockEvent.h"
#include "ila/event/explosion/ExplosionExperienceBlockEvent.h"
#include "ila/event/explosion/ExplosionFlameEvent.h"
#include "ila/event/explosion/ExplosionKnockbackEntityEvent.h"
#include "ila/event/explosion/ExplosionLootBlockEvent.h"
#include "ila/event/explosion/ExplosionParticleEvent.h"
#include "ila/event/explosion/ExplosionProcessBlockEvent.h"
#include "ila/event/explosion/ExplosionProcessEntityEvent.h"
#include "ila/event/explosion/ExplosionSoundEvent.h"
#include "patch_mc/deps/shared_types/legacy/LevelEvent.i.h"
#include "patch_mc/deps/shared_types/legacy/LevelSoundEvent.i.h"
#include "patch_mc/world/events/EventCoordinator.i.h"
#include <algorithm>
#include <array>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstdlib>
#include <ll/api/base/Containers.h>
#include <ll/api/base/StdInt.h>
#include <ll/api/event/world/WorldEvent.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>
#include <mc/deps/core/math/IRandom.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/core/utility/optional_ref.h>
#include <mc/deps/ecs/gamerefs_entity/EntityContext.h>
#include <mc/deps/ecs/gamerefs_entity/GameRefsEntity.h>
#include <mc/deps/game_refs/WeakRef.h>
#include <mc/deps/shared_types/legacy/LevelEvent.h>
#include <mc/deps/shared_types/legacy/LevelSoundEvent.h>
#include <mc/deps/shared_types/legacy/actor/ActorDamageCause.h>
#include <mc/entity/components/ServerMovement.h>
#include <mc/entity/components/WindChargeKnockbackComponent.h>
#include <mc/entity/components_json_legacy/ExplodeComponent.h>
#include <mc/gameplayhandlers/CoordinatorResult.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/network/packet/LevelEventGenericPacket.h>
#include <mc/util/Random.h>
#include <mc/util/Randomize.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorCategory.h>
#include <mc/world/actor/ActorDamageByActorSource.h>
#include <mc/world/actor/ActorDamageByChildActorSource.h>
#include <mc/world/actor/ActorDamageSource.h>
#include <mc/world/actor/ActorSoundIdentifier.h>
#include <mc/world/actor/ActorType.h>
#include <mc/world/actor/KnockbackRules.h>
#include <mc/world/actor/item/ExperienceOrb.h>
#include <mc/world/attribute/Attribute.h>
#include <mc/world/attribute/AttributeInstance.h>
#include <mc/world/attribute/AttributeInstanceConstRef.h>
#include <mc/world/attribute/SharedAttributes.h>
#include <mc/world/events/BlockEventCoordinator.h>
#include <mc/world/events/EventRef.h>
#include <mc/world/events/ExplosionStartedEvent.h>
#include <mc/world/events/MutableBlockGameplayEvent.h>
#include <mc/world/events/gameevents/GameEventRegistry.h>
#include <mc/world/item/ItemStack.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/Explosion.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/ParticlesBlockExplosionEvent.h>
#include <mc/world/level/block/ActorChangeContext.h>
#include <mc/world/level/block/BedrockBlockNames.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/BlockChangeContext.h>
#include <mc/world/level/block/BlockType.h>
#include <mc/world/level/block/ResourceDrops.h>
#include <mc/world/level/block/ResourceDropsCause.h>
#include <mc/world/level/block/ResourceDropsContext.h>
#include <mc/world/level/block/VanillaBlockTypeIds.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/level/levelgen/structure/BoundingBox.h>
#include <mc/world/level/material/Material.h>
#include <mc/world/level/material/MaterialType.h>
#include <mc/world/level/storage/GameRuleId.h>
#include <mc/world/level/storage/GameRules.h>
#include <optional>
#include <ranges>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ila::explosion {

struct ExplosionReflection {
    Vec3                                 pos;
    float                                radius;
    std::unordered_set<BlockPos>         $affected_blocks;
    bool                                 fire;
    bool                                 breaking;
    bool                                 allow_underwater;
    bool                                 can_toggle_blocks;
    float                                damage_scaling;
    bool                                 ignore_block_explosion_resistance;
    SharedTypes::Legacy::LevelEvent      particle_type;
    SharedTypes::Legacy::LevelSoundEvent sound_explosion_type;
    int64                                source_id;
    uintptr_t                            $region;
    float                                max_resistance;
    std::optional<bool>                  in_water_override;
    std::optional<int>                   total_damage_override;
    float                                knockback_scaling;
};

static_assert(
    sizeof(Explosion) == sizeof(ExplosionReflection),
    "Explosion and ExplosionReflection are not the same size"
);
static_assert(
    alignof(Explosion) == alignof(ExplosionReflection),
    "Explosion and ExplosionReflection are not the same alignment"
);

void ExplosionEvent::serialize(CompoundTag& nbt) const {
    WorldEvent::serialize(nbt);
    ll::reflection::serialize_to(nbt["explosion"], reinterpret_cast<ExplosionReflection&>(mExplosion)).value();
}

void ExplosionEvent::deserialize(CompoundTag const& nbt) {
    WorldEvent::deserialize(nbt);
    ll::reflection::deserialize(reinterpret_cast<ExplosionReflection&>(mExplosion), nbt["explosion"]).value();
}

LL_TYPE_INSTANCE_HOOK(ExplosionEventHook, HookPriority::Low, Explosion, &Explosion::explode, bool, IRandom& random) {
    if (mRadius == 0.0f) return false;

    if (eventPromise(ExplodingEvent{*this}).publish()) return false;

    auto& level   = mRegion.mLevel;
    auto  source  = optional_ref{level.fetchEntity(mSourceID, false)};
    auto  inWater = [&]() {
        if (*mInWaterOverride) return **mInWaterOverride;
        if (source) return source->isInWater();
        return mRegion.getLiquidBlock(BlockPos{mPos}).mBlockType->mMaterial.mType == MaterialType::Water;
    }();
    auto& airBlock       = *Block::tryGetFromRegistry(BedrockBlockNames::Air());
    auto& fireBlock      = *Block::tryGetFromRegistry(VanillaBlockTypeIds::Fire());
    auto  forceFullDrops = [&]() {
        if (!source) return false;
        if (auto type = source->getEntityTypeId(); type == ActorType::PrimedTnt || type == ActorType::MinecartTNT) {
            if (auto& gameRules = level.getGameRules(); gameRules.getBool(
                    GameRuleId{std::to_underlying(GameRules::GameRulesIndex::TntExplosionDropDecay)},
                    false
                )) {
                return true;
            }
        }
        return false;
    }();

    if (auto& block = mRegion.getBlock(BlockPos{mPos}); block.mBlockType->mMaterial.mSolid && source) {
        auto posDelta = *source->mBuiltInComponents->mStateVectorComponent->mPosDelta;
        if (auto numSteps = std::min(static_cast<int>(std::ceil(posDelta.length())), 5); numSteps > 0) {
            Vec3 normalizedDelta = posDelta.normalized();
            Vec3 currentPos      = mPos;

            if (std::ranges::any_of(std::views::iota(numSteps), [&](auto) {
                currentPos -= normalizedDelta;
                return !mRegion.getBlock({currentPos}).mBlockType->mMaterial.mSolid;
            })) {
                eventPromise(ExplosionCollidingEvent{*this, mPos, currentPos})
                    .onSuccess([&] {
                    std::swap(*mPos, currentPos);
                    inWater =
                        mRegion.getLiquidBlock(BlockPos{currentPos}).mBlockType->mMaterial.mType == MaterialType::Water;
                })
                    .onSuccessEvent(ExplosionCollidedEvent{*this, currentPos, mPos})
                    .publish();
            }
        }
    }

    if ((mBreaking || mCanToggleBlocks) && (!inWater || mAllowUnderwater)) {
        constexpr float stepScale = 2.0f / 15.0f; // 0.13333334f

        for (auto boundaryPos : BoundingBox{0, 16}.forEachPos()) {
            if (std::ranges::all_of(reinterpret_cast<std::array<int, 3>&>(boundaryPos), [](auto value) {
                return value != 0 && value != 15;
            })) {
                continue;
            }

            auto direction  = Vec3{boundaryPos} * stepScale - 1.0f;
            direction      /= direction.length();

            Vec3 currentPos = mPos;

            for (auto remainingStrength  = mRadius * (random.nextFloat() * 0.6f + 0.7f); remainingStrength > 0.0f;
                 remainingStrength      -= 0.75f * 0.3f) {
                BlockPos blockPos(currentPos);

                if (!mAllowUnderwater) {
                    if (auto& extraBlock = mRegion.getExtraBlock(blockPos);
                        extraBlock.mBlockType->mMaterial.mType == MaterialType::Water) {
                        remainingStrength = 0.0f;
                    }
                }

                auto& currentBlock = mRegion.getBlock(blockPos);

                if (currentBlock.isAir()
                    || (mAllowUnderwater ? currentBlock.mBlockType->mMaterial.mType == MaterialType::Water : false)) {
                    if (mFire && mRegion.getBlock(blockPos.add({0, -1, 0})).mBlockType->mMaterial.mSolid) {
                        mAffectedBlocks->emplace(blockPos);
                    }
                } else {
                    auto blockResistance =
                        mIgnoreBlockExplosionResistance ? 0.0f : currentBlock.mDirectData->mExplosionResistance;
                    if (source && source->canDestroyBlock(currentBlock)) {
                        blockResistance = std::min(mMaxResistance * 0.2f, blockResistance);
                    }
                    remainingStrength -= (blockResistance * 0.3f + 0.3f) * 0.3f;
                    if (remainingStrength > 0.0f) mAffectedBlocks->emplace(blockPos);
                }

                currentPos += direction * 0.3f;
            }
        }
    }

    if (!level.isClientSide()) { // 给脚本层推送事件
        struct {
            std::unordered_set<BlockPos> mBlocks;
            Dimension&                   mDimension;
            WeakRef<EntityContext>       mSource;
        } event{mAffectedBlocks, mRegion.mDimension, source ? source->getWeakEntity() : WeakRef<EntityContext>{}};
        static_assert(sizeof(event) == sizeof(ExplosionStartedEvent));
        EventRef<MutableBlockGameplayEvent<CoordinatorResult>> eventRef{
            reinterpret_cast<ExplosionStartedEvent&>(event)
        };
        if (level.getBlockEventCoordinator().sendEvent(eventRef) == CoordinatorResult::Cancel) return false;
        mAffectedBlocks = std::move(event.mBlocks);
    }

    auto doubleRadius = mRadius * 2.0f;
    // clang-format off
    auto sourcePos = source.transform([&](Actor& entity) {
        auto type = entity.getEntityTypeId();
        return type == ActorType::WindChargeProjectile || type == ActorType::BreezeWindChargeProjectile ? *mPos : entity.getEyePos();
    }).value_or(mPos);
    // clang-format on
    for (auto entity : _getActorsInRange(source, doubleRadius)) { // 处理实体
        if (entity->isSpectator() || eventPromise(ExplosionProcessEntityingEvent{*this, *entity}).publish()) {
            continue;
        }

        auto& aabb      = *entity->mBuiltInComponents->mAABBShapeComponent->mAABB;
        auto  entityPos = *entity->mBuiltInComponents->mStateVectorComponent->mPos;
        entityPos.y     = aabb.min.y;
        auto distance   = static_cast<float>((entityPos - sourcePos).length());

        if (entity->getEntityTypeId() != ActorType::Player) {
            distance = std::max(0.0f, distance - entity->mBuiltInComponents->mAABBShapeComponent->mBBDim->x * 0.5f);
        }

        auto normalizedDistance = distance / doubleRadius;
        if (normalizedDistance > 1.0f) continue;
        auto pow         = (1.0f - normalizedDistance) * mRegion.getSeenPercent(mPos, aabb) * mKnockbackScaling;
        auto totalDamage = *mTotalDamageOverride
                             ? static_cast<float>(**mTotalDamageOverride)
                             : ((pow * pow * 3.5f + pow * 0.5f * 7.0f) * doubleRadius + 1.0f) * mDamageScaling;

        { // 伤害实体
            auto [damageSource, damaging, knockback] = [&]() -> std::tuple<ActorDamageSource, bool, bool> {
                using ActorDamageCause = SharedTypes::Legacy::ActorDamageCause;

                // 情况1：实体正在格挡 → 伤害减半，原因取决于伤害来源
                if (entity->isBlocking()) {
                    pow *= 0.5f;
                    // clang-format off
                    ActorDamageSource damageSource;
                    damageSource.mCause = source.and_then([](Actor& source) -> std::optional<ActorDamageCause> {
                        if (source.hasCategory(ActorCategory::Mob) || source.getEntityTypeId() == ActorType::PrimedTnt) {
                            return ActorDamageCause::EntityExplosion;
                        }
                        return std::nullopt;
                    }).value_or(ActorDamageCause::BlockExplosion);
                    return {damageSource, false, false};   // 不造成伤害击退
                    // clang-format on
                }

                // 情况2：伤害缩放接近零 → 无伤害
                if (std::abs(mDamageScaling) <= 0.0000099999997f) {
                    return {};
                }

                // 情况3：没有伤害来源 → 使用方块爆炸原因
                if (!source) {
                    ActorDamageSource damageSource;
                    damageSource.mCause = ActorDamageCause::BlockExplosion;
                    return {damageSource, true, false};
                }

                // 情况4：水中且禁止水下爆炸 → 伤害置零
                if (inWater && !mAllowUnderwater) {
                    totalDamage = 0.0f;
                }

                // 情况5：存在子伤害来源（如点燃 TNT 的实体）
                if (auto* childSource = level.fetchEntity(source->getSourceUniqueID(), false); childSource) {
                    return {
                        ActorDamageByChildActorSource{*source, *childSource, ActorDamageCause::EntityExplosion},
                        true,
                        false
                    };
                }

                // 情况6：伤害来源是生物或点燃的 TNT → 使用实体爆炸原因
                if (source->hasCategory(ActorCategory::Mob) || source->getEntityTypeId() == ActorType::PrimedTnt) {
                    return {
                        ActorDamageByActorSource{*source, ActorDamageCause::EntityExplosion},
                        true,
                        false
                    };
                }

                // 情况7：默认 → 使用方块爆炸原因
                ActorDamageSource damageSource;
                damageSource.mCause = ActorDamageCause::BlockExplosion;
                return {damageSource, true, false};
            }();
            // clang-format off
            if (!eventPromise(
                    ExplosionDamageEntityingEvent{*this, *entity, damageSource, totalDamage, damaging, knockback}
                ).publish() && totalDamage > 0.0f
            ) {
                // clang-format on
                entity->hurt(damageSource, totalDamage, damaging, knockback);

                eventPromise(
                    ExplosionDamageEntityedEvent{*this, *entity, damageSource, totalDamage, damaging, knockback}
                )
                    .publish();
            }
        }

        if ([&]() { // 击退实体
            if (auto attribute = entity->getAttribute(SharedAttributes::KNOCKBACK_RESISTANCE()); attribute.mPtr) {
                return attribute.mPtr->mCurrentValue;
            }
            return 0.0f;
        }() < 1.0f) {
            auto type              = entity->getEntityTypeId();
            auto actorKnockbackPos = type == ActorType::PrimedTnt
                                       ? *entity->mBuiltInComponents->mStateVectorComponent->mPos
                                       : getEyePos(*entity);

            auto direction = actorKnockbackPos - mPos;
            if (auto directionLength = direction.length(); directionLength >= 0.0001f) {
                direction *= 1.0f / directionLength;
            } else {
                direction = {};
            }

            Vec3 knockback = direction * KnockbackRules::getScaledKnockbackForce(*entity, pow);

            eventPromise(ExplosionKnockbackEntityingEvent{*this, *entity, knockback}).onCancel([&] {
                knockback = {};
            }).publish();

            if (knockback.x != 0.0f || knockback.y != 0.0f || knockback.z != 0.0f) {
                if (type == ActorType::Player && source) {
                    auto explodeComp = source->mEntityContext->tryGetComponent<ExplodeComponent>();
                    if (explodeComp && explodeComp->mNegatesFallDamage) {
                        auto& windComp = entity->mEntityContext->getOrAddComponent<WindChargeKnockbackComponent>();
                        windComp.mKnockbackStartYCoordinate =
                            entity->mBuiltInComponents->mStateVectorComponent->mPos->y;
                        windComp.mTicksAfterAddition = 0;
                    }
                }

                *entity->mBuiltInComponents->mStateVectorComponent->mPosDelta += knockback;
                ServerMovement::notifyOfServerInitiatedMotion(entity->mEntityContext);

                eventPromise(ExplosionKnockbackEntityedEvent{*this, *entity, knockback}).publish();
            }
        }

        eventPromise(ExplosionProcessEntityedEvent{*this, *entity}).publish();
    }

    if (mRadius > 0.0f) { // 粒子效果和声音
        eventPromise(ExplosionSoundingEvent{*this})
            .onSuccess([&] { level.broadcastSoundEvent(mRegion, mSoundExplosionType, mPos, -1, {}, false); })
            .onSuccessEvent(ExplosionSoundedEvent{*this})
            .publish();
        eventPromise(ExplosionParticlingEvent{*this, mPos, mParticleType})
            .onSuccess([&] { level.broadcastLocalEvent(mRegion, mParticleType, mPos, static_cast<int>(mRadius)); })
            .onSuccessEvent(ExplosionParticledEvent{*this, mPos, mParticleType})
            .publish();
    }

    mRegion.postGameEvent(source, GameEventRegistry::explode(), mPos, nullptr);

    BlockPos minPos{INT_MAX};
    BlockPos maxPos{INT_MIN};
    bool     blockChangable{false};

    { // 方块相关处理
        ParticlesBlockExplosionEvent particlesEvent{mRadius, mPos, {}};
        // key: blockPos, isExtraBlock  value: block, itemStacks
        ll::OrderedMap<std::pair<BlockPos, bool>, std::pair<Block const*, std::vector<ItemStack>>> blockDrops;

        for (auto& blockPos : *mAffectedBlocks) {
            auto& block      = mRegion.getBlock(blockPos);
            bool  isAirBlock = block.isAir();
            auto& extraBlock = mRegion.getExtraBlock(blockPos);

            if (!level.isClientSide() && mCanToggleBlocks) { // 激活方块
                if (!source || source->getEntityTypeId() != ActorType::BreezeWindChargeProjectile) {
                    block.mBlockType->_onHitByActivatingAttack(mRegion, blockPos, source);
                }
            }

            if (mBreaking) { // 方块被爆炸破坏
                if (!random.nextInt(8)) {
                    auto pos = Vec3{blockPos};
                    eventPromise(
                        ExplosionParticlingEvent{*this, pos, SharedTypes::Legacy::LevelEvent::ParticlesBlockExplosion}
                    )
                        .onSuccess([&] {
                        particlesEvent.mPositions.emplace_back(pos);
                    }).publish();
                }

                if (!block.isAir()) {
                    if (!level.isClientSide()) {
                        ResourceDropsContext resourceDropsContext{
                            ResourceDropsCause::Explosion,
                            forceFullDrops ? 1.0f : mRadius,
                            ItemStack::EMPTY_ITEM(),
                            blockPos,
                            mRegion.mDimension.mId,
                            mRegion
                        };

                        Randomize randomize(level.getRandom());

                        auto destroy = [&](Block const& block, bool isExtraBlock) {
                            // clang-format off
                            if (block.isAir() || eventPromise(ExplosionProcessBlockingEvent{*this, blockPos, block, isExtraBlock}).publish()) return;
                            // clang-format on
                            auto resources = block.mBlockType->getResourceDrops(block, randomize, resourceDropsContext);
                            { // 获取并保存方块掉落物
                                // clang-format off
                                auto event = !eventPromise(
                                    ExplosionLootingBlockEvent{
                                        *this,
                                        blockPos,
                                        block,
                                        isExtraBlock,
                                        *resources.mItems
                                    }
                                ).publish();
                                // clang-format on
                                if (!event && !resources.mItems->empty()) {
                                    blockDrops.insert({
                                        {blockPos, isExtraBlock                },
                                        {&block,   std::move(*resources.mItems)}
                                    });
                                }
                            }
                            { // 生成经验球
                                // clang-format off
                                auto event = eventPromise(
                                    ExplosionExperienceBlockingEvent{
                                        *this,
                                        blockPos,
                                        block,
                                        isExtraBlock,
                                        resources.mExperienceCount
                                    }
                                ).publish();
                                // clang-format on
                                if (!event && resources.mExperienceCount > 0) {
                                    ExperienceOrb::spawnOrbs(
                                        mRegion,
                                        blockPos,
                                        resources.mExperienceCount,
                                        ExperienceOrb::DropType::FromBlock,
                                        nullptr
                                    );

                                    eventPromise(
                                        ExplosionExperienceBlockedEvent{
                                            *this,
                                            blockPos,
                                            block,
                                            isExtraBlock,
                                            resources.mExperienceCount
                                        }
                                    ).publish();
                                }
                            }
                            {
                                if (!eventPromise(ExplosionDestroyBlockingEvent{*this, blockPos, block, isExtraBlock}).publish()) {
                                    if (isExtraBlock) {
                                        mRegion.setExtraBlock(blockPos, airBlock, 3);
                                    } else {
                                        BlockChangeContext changeCtx{false};
                                        changeCtx.mContextSource = ActorChangeContext{source};
                                        mRegion.setBlock(blockPos, airBlock, 3, nullptr, changeCtx);
                                        isAirBlock = true;
                                    }
                                    block.mBlockType->spawnAfterBreak(mRegion, block, blockPos, resourceDropsContext);
                                    eventPromise(
                                        ExplosionDestroyBlockedEvent{*this, blockPos, block, isExtraBlock}
                                    ).publish();
                                }
                            }
                            block.mBlockType->onExploded(mRegion, blockPos, source);
                            eventPromise(ExplosionProcessBlockedEvent{*this, blockPos, block, isExtraBlock}).publish();
                        };

                        destroy(block, false);
                        destroy(extraBlock, true);
                    }

                    minPos         = std::min(minPos, blockPos);
                    maxPos         = std::max(maxPos, blockPos);
                    blockChangable = true;
                }

                if (!level.isClientSide()) {
                    for (auto* listener : level.getBlockEventCoordinator().mListeners) {
                        if (!listener) continue;
                        listener->onBlockExploded(mRegion.mDimension, blockPos, block, source);
                    }
                }
            }

            if (!level.isClientSide()) { // 生成方块被爆炸破坏粒子
                LevelEventGenericPacket{
                    {std::to_underlying(SharedTypes::Legacy::LevelEvent::ParticlesBlockExplosion),
                     std::move(*particlesEvent.save())}
                }.sendTo(*mPos, mRegion.mDimension.mId);
                for (auto& pos : particlesEvent.mPositions) {
                    eventPromise(
                        ExplosionParticledEvent{*this, pos, SharedTypes::Legacy::LevelEvent::ParticlesBlockExplosion}
                    )
                        .publish();
                }
            }

            if (mFire && isAirBlock) { // 生成火焰方块
                if (mRegion.getBlock(blockPos.add({0, -1, 0})).mBlockType->mMaterial.mSolid && !random.nextInt(3)) {
                    auto pos   = blockPos;
                    auto event = ExplosionFlamingEvent{*this, pos};
                    if (!event.isCancelled()) {
                        BlockChangeContext changeCtx{false};
                        changeCtx.mContextSource = ActorChangeContext{source};
                        mRegion.setBlock(pos, fireBlock, 3, nullptr, changeCtx);
                        eventPromise(ExplosionFlamedEvent{*this, pos}).publish();
                    }

                    minPos = std::min(minPos, pos);
                    maxPos = std::max(maxPos, pos);
                }
            }
        }

        /*
        生成方块掉落物
        物品合并逻辑：
            - 限制单个物品掉落物品数量 (计算公式为: 16 * (length / 10 + 1))
            - 其中length为items当前的长度

            - 然后就是正常的物品合并操作
            - 只不过最大堆叠数量为 min(上面计算的限制, 物品堆叠上限)

            - 最后一点，不检测坐标，无视距离的
            - 也就是说哪个方块先被遍历，就掉落在哪
        */
        if (!blockDrops.empty()) {
            std::vector<std::pair<ItemStack, BlockPos>> items;
            for (auto& [key, value] : blockDrops) {
                for (auto& itemStack : value.second) {
                    _addOrMergeItemStack(itemStack, key.first, items);
                }
            }
            for (auto& [itemStack, pos] : items) {
                BlockType::popResource(mRegion, pos, itemStack);
            }
            for (auto& [key, value] : blockDrops) {
                eventPromise(ExplosionLootedBlockEvent{*this, key.first, *value.first, key.second, value.second})
                    .publish();
            }
        }
    }


    if (blockChangable) { // 触发方块更新事件
        auto area = BoundingBox{minPos.add(-1), maxPos.add(1)};
        for (auto* listener : *mRegion.mListeners) {
            if (!listener) continue;
            listener->onAreaChanged(mRegion, area.min, area.max);
        }
    }

    eventPromise(ExplodedEvent{*this}).publish();
    return true;
}

EventHook(ExplodingEvent, ExplodedEvent, <ExplosionEventHook>);
EventHook(ExplosionCollidingEvent, ExplosionCollidedEvent, <ExplosionEventHook>);
EventHook(ExplosionDamageEntityingEvent, ExplosionDamageEntityedEvent, <ExplosionEventHook>);
EventHook(ExplosionDestroyBlockingEvent, ExplosionDestroyBlockedEvent, <ExplosionEventHook>);
EventHook(ExplosionExperienceBlockingEvent, ExplosionExperienceBlockedEvent, <ExplosionEventHook>);
EventHook(ExplosionFlamingEvent, ExplosionFlamedEvent, <ExplosionEventHook>);
EventHook(ExplosionKnockbackEntityingEvent, ExplosionKnockbackEntityedEvent, <ExplosionEventHook>);
EventHook(ExplosionLootingBlockEvent, ExplosionLootedBlockEvent, <ExplosionEventHook>);
EventHook(ExplosionParticlingEvent, ExplosionParticledEvent, <ExplosionEventHook>);
EventHook(ExplosionProcessBlockingEvent, ExplosionProcessBlockedEvent, <ExplosionEventHook>);
EventHook(ExplosionProcessEntityingEvent, ExplosionProcessEntityedEvent, <ExplosionEventHook>);
EventHook(ExplosionSoundingEvent, ExplosionSoundedEvent, <ExplosionEventHook>);

} // namespace ila::explosion