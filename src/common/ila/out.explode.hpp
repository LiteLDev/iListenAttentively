#pragma once
#include <mc/deps/core/math/IRandom.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/core/string/HashedString.h>
#include <mc/deps/ecs/gamerefs_entity/EntityContext.h>
#include <mc/deps/game_refs/WeakRef.h>
#include <mc/deps/shared_types/legacy/LevelEvent.h>
#include <mc/deps/shared_types/legacy/LevelSoundEvent.h>
#include <mc/deps/shared_types/legacy/actor/ActorDamageCause.h>
#include <mc/deps/vanilla_components/AABBShapeComponent.h>
#include <mc/deps/vanilla_components/StateVectorComponent.h>
#include <mc/entity/components/ServerMovement.h>
#include <mc/entity/components/WindChargeKnockbackComponent.h>
#include <mc/entity/components_json_legacy/ExplodeComponent.h>
#include <mc/gameplayhandlers/CoordinatorResult.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/network/packet/LevelEventGenericPacket.h>
#include <mc/network/packet/LevelEventGenericPacketPayload.h>
#include <mc/util/Random.h>
#include <mc/util/Randomize.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorCategory.h>
#include <mc/world/actor/ActorDamageByActorSource.h>
#include <mc/world/actor/ActorDamageByChildActorSource.h>
#include <mc/world/actor/ActorDamageSource.h>
#include <mc/world/actor/ActorSoundIdentifier.h>
#include <mc/world/actor/ActorType.h>
#include <mc/world/actor/BuiltInActorComponents.h>
#include <mc/world/actor/KnockbackRules.h>
#include <mc/world/attribute/Attribute.h>
#include <mc/world/attribute/AttributeInstance.h>
#include <mc/world/attribute/BaseAttributeMap.h>
#include <mc/world/attribute/SharedAttributes.h>
#include <mc/world/events/BlockEventCoordinator.h>
#include <mc/world/events/BlockEventListener.h>
#include <mc/world/events/EventRef.h>
#include <mc/world/events/ExplosionStartedEvent.h>
#include <mc/world/events/MutableBlockGameplayEvent.h>
#include <mc/world/events/gameevents/GameEvent.h>
#include <mc/world/events/gameevents/GameEventRegistry.h>
#include <mc/world/item/ItemStack.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/BlockSourceListener.h>
#include <mc/world/level/Explosion.h>
#include <mc/world/level/ILevel.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/ParticlesBlockExplosionEvent.h>
#include <mc/world/level/block/ActorChangeContext.h>
#include <mc/world/level/block/BedrockBlockNames.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/BlockChangeContext.h>
#include <mc/world/level/block/BlockType.h>
#include <mc/world/level/block/ResourceDropsCause.h>
#include <mc/world/level/block/ResourceDropsContext.h>
#include <mc/world/level/block/VanillaBlockTypeIds.h>
#include <mc/world/level/block/components/BlockComponentDirectData.h>
#include <mc/world/level/block/registry/BlockTypeRegistry.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/level/material/Material.h>
#include <mc/world/level/material/MaterialType.h>
#include <mc/world/level/storage/GameRuleId.h>
#include <mc/world/level/storage/GameRules.h>
#include <mc/world/phys/AABB.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ll/api/base/StdInt.h>
#include <optional>
#include <unordered_set>
#include <utility>
#include <vector>

bool Explosion::explode(IRandom& random) {
    using namespace SharedTypes::Legacy;
    if (mRadius == 0.0f) {
        return false;
    }

    Level& level = mRegion.getLevel();
    float  org   = mRadius;

    Actor* source = level.fetchEntity(mSourceID, false);

    bool inWater;
    if (mInWaterOverride.get().has_value()) {
        inWater = mInWaterOverride.get().value();
    } else if (source) {
        inWater = source->isInWater();
    } else {
        BlockPos    bpos(mPos);
        auto const& liquidBlock = mRegion.getLiquidBlock(bpos);
        inWater                 = liquidBlock.mBlockType->mMaterial.mType == MaterialType::Water;
    }

    BlockPos    bpos(mPos);
    auto const& block = mRegion.getBlock(bpos);
    if (block.mBlockType->mMaterial.mSolid && source) {
        Vec3        originalPos = mPos;
        Vec3 const& posDelta    = source->mBuiltInComponents.get().mStateVectorComponent->mPosDelta;
        float       deltaLen    = std::sqrt(posDelta.lengthSquared());
        int         numSteps    = std::min(static_cast<int>(std::ceil(deltaLen)), 5);
        if (numSteps > 0) {
            Vec3 normalizedDelta = posDelta.normalized();
            bool nonSolidFound   = false;
            for (int i = 0; i < numSteps; ++i) {
                mPos.get() -= normalizedDelta;
                BlockPos stepPos(mPos);
                if (!mRegion.getBlock(stepPos).mBlockType->mMaterial.mSolid) {
                    nonSolidFound = true;
                    break;
                }
            }
            if (nonSolidFound) {
                BlockPos    newPos(mPos);
                auto const& liq = mRegion.getLiquidBlock(newPos);
                inWater         = liq.mBlockType->mMaterial.mType == MaterialType::Water;
            } else {
                mPos = originalPos;
            }
        }
    }


    if ((mBreaking || mCanToggleBlocks) && (!inWater || mAllowUnderwater)) {
        constexpr float stepScale = 2.0f / 15.0f; // 0.13333334f
        for (int xx = 0; xx < 16; ++xx) {
            for (int yy = 0; yy < 16; ++yy) {
                for (int zz = 0; zz < 16; ++zz) {
                    if (xx != 0 && xx != 15 && yy != 0 && yy != 15 && zz != 0 && zz != 15) {
                        continue;
                    }

                    float xd  = static_cast<float>(xx) * stepScale - 1.0f;
                    float yd  = static_cast<float>(yy) * stepScale - 1.0f;
                    float zd  = static_cast<float>(zz) * stepScale - 1.0f;
                    float d   = std::sqrt(xd * xd + yd * yd + zd * zd);
                    xd       /= d;
                    yd       /= d;
                    zd       /= d;

                    float remainingPower = mRadius * (random.nextFloat() * 0.6f + 0.7f);
                    Vec3  p              = mPos;

                    while (remainingPower > 0.0f) {
                        BlockPos t(p);

                        if (!mAllowUnderwater) {
                            auto const& extraBlock = mRegion.getExtraBlock(t);
                            if (extraBlock.mBlockType->mMaterial.mType == MaterialType::Water) {
                                remainingPower = 0.0f;
                            }
                        }

                        auto const& block     = mRegion.getBlock(t);
                        auto const& blockType = *block.mBlockType;

                        bool isAir        = (blockType.mNameInfo.get().mFullName == BedrockBlockNames::Air());
                        bool isWaterBlock = false;
                        if (mAllowUnderwater) {
                            auto const& fullName = blockType.mNameInfo.get().mFullName.get();
                            isWaterBlock         = (fullName.getHash() == VanillaBlockTypeIds::FlowingWater().getHash())
                                        || (fullName.getHash() == VanillaBlockTypeIds::Water().getHash());
                        }

                        if (isAir || (mAllowUnderwater && isWaterBlock)) {
                            if (mFire) {
                                BlockPos below(t.x, t.y - 1, t.z);
                                if (mRegion.getBlock(below).mBlockType->mMaterial.mSolid) {
                                    mAffectedBlocks.get().emplace(t);
                                }
                            }
                        } else {
                            float effectiveResistance = 0.0f;
                            if (!mIgnoreBlockExplosionResistance) {
                                effectiveResistance = block.mDirectData.get().mExplosionResistance;
                            }
                            if (source && source->canDestroyBlock(block)) {
                                effectiveResistance = std::min(mMaxResistance * 0.2f, effectiveResistance);
                            }
                            remainingPower -= (effectiveResistance * 0.3f + 0.3f) * 0.3f;
                            if (remainingPower > 0.0f) {
                                mAffectedBlocks.get().emplace(t);
                            }
                        }

                        p.x            += xd * 0.3f;
                        p.y            += yd * 0.3f;
                        p.z            += zd * 0.3f;
                        remainingPower -= 0.75f * 0.3f;
                    }
                }
            }
        }
    }

    if (!level.isClientSide()) {
        WeakRef<EntityContext> sourceRef;
        if (source) {
            sourceRef = source->getWeakEntity();
        }

        ExplosionStartedEvent explosionStartedEvent;
        explosionStartedEvent.mBlocks    = mAffectedBlocks;
        explosionStartedEvent.mDimension = mRegion.getDimension();
        explosionStartedEvent.mSource    = sourceRef;

        auto&                                                  blockEventCoordinator = level.getBlockEventCoordinator();
        EventRef<MutableBlockGameplayEvent<CoordinatorResult>> eventRef(explosionStartedEvent);
        CoordinatorResult result = blockEventCoordinator.sendEvent(std::move(eventRef));
        if (result == CoordinatorResult::Cancel) {
            return false;
        }
        mAffectedBlocks = std::move(explosionStartedEvent.mBlocks);
    }

    float                              doubleRadius = mRadius * 2.0f;
    std::vector<gsl::not_null<Actor*>> actors       = _getActorsInRange(source, doubleRadius);

    Vec3 sourcePos;
    if (source) {
        ActorType sourceType = source->getEntityTypeId();
        if (sourceType == ActorType::WindChargeProjectile || sourceType == ActorType::BreezeWindChargeProjectile) {
            sourcePos = mPos;
        } else {
            sourcePos = getEyePos(*source);
        }
    } else {
        sourcePos = mPos;
    }

    for (auto& actorRef : actors) {
        Actor* actor = actorRef;

        if (actor->isSpectator()) {
            continue;
        }

        auto const& aabb = actor->mBuiltInComponents.get().mAABBShapeComponent->mAABB.get();

        Vec3 const& actorPos = actor->mBuiltInComponents.get().mStateVectorComponent->mPos.get();
        Vec3        entityPos(actorPos.x, aabb.min.y, actorPos.z);

        Vec3  entityDiff = entityPos - sourcePos;
        float dist       = std::sqrt(entityDiff.lengthSquared());

        float entityRadius = actor->mBuiltInComponents.get().mAABBShapeComponent->mBBDim.get().x * 0.5f;

        if (actor->getEntityTypeId() != ActorType::Player) {
            dist = std::max(0.0f, dist - entityRadius);
        }

        float normalizedDist = dist / doubleRadius;
        if (normalizedDist > 1.0f) {
            continue;
        }

        float pow = (1.0f - normalizedDist) * mRegion.getSeenPercent(mPos, aabb) * mKnockbackScaling;

        float totalDamage = ((pow * pow * 3.5f + pow * 0.5f * 7.0f) * doubleRadius + 1.0f) * mDamageScaling;

        if (mTotalDamageOverride.get().has_value()) {
            totalDamage = static_cast<float>(mTotalDamageOverride.get().value());
        }

        if (actor->isBlocking()) {
            pow *= 0.5f;

            ActorDamageCause cause = ActorDamageCause::BlockExplosion;
            if (source) {
                if (source->hasCategory(ActorCategory::Mob) || source->getEntityTypeId() == ActorType::PrimedTnt) {
                    cause = ActorDamageCause::EntityExplosion;
                }
            }
            ActorDamageSource damageSource;
            damageSource.mCause = cause;
            actor->hurt(damageSource, totalDamage, false, false);
        } else if (std::abs(mDamageScaling) > 0.0000099999997f) {
            if (!source) {
                ActorDamageSource damageSource;
                damageSource.mCause = ActorDamageCause::BlockExplosion;
                actor->hurt(damageSource, totalDamage, true, false);
            } else {
                if (inWater && !mAllowUnderwater) {
                    totalDamage = 0.0f;
                }

                Level&        lvl         = mRegion.getLevel();
                ActorUniqueID childId     = source->getSourceUniqueID();
                Actor*        childSource = lvl.fetchEntity(childId, false);

                if (childSource) {
                    ActorDamageByChildActorSource dmgSrc(*source, *childSource, ActorDamageCause::EntityExplosion);
                    actor->hurt(dmgSrc, totalDamage, true, false);
                } else if (source->hasCategory(ActorCategory::Mob)
                           || source->getEntityTypeId() == ActorType::PrimedTnt) {
                    ActorDamageByActorSource dmgSrc(*source, ActorDamageCause::EntityExplosion);
                    actor->hurt(dmgSrc, totalDamage, true, false);
                } else {
                    ActorDamageSource dmgSrc;
                    dmgSrc.mCause = ActorDamageCause::BlockExplosion;
                    actor->hurt(dmgSrc, totalDamage, true, false);
                }
            }
        }

        auto const& attributes               = actor->getAttributes();
        float       knockbackResistanceValue = 0.0f;
        {
            uint        krId = SharedAttributes::KNOCKBACK_RESISTANCE().mIDValue;
            auto const& keys = *reinterpret_cast<std::vector<uint> const*>(
                reinterpret_cast<char const*>(&((*attributes).mInstanceMap.get()))
            );
            auto const& values = *reinterpret_cast<std::vector<AttributeInstance> const*>(
                reinterpret_cast<char const*>(&((*attributes).mInstanceMap.get())) + sizeof(std::vector<uint>)
            );
            auto it = std::lower_bound(keys.begin(), keys.end(), krId);
            if (it != keys.end() && *it == krId) {
                size_t idx               = static_cast<size_t>(it - keys.begin());
                knockbackResistanceValue = values[idx].mCurrentValue;
            }
        }

        if (knockbackResistanceValue < 1.0f) {
            ActorType actorType = actor->getEntityTypeId();
            Vec3      actorKnockbackPos;
            if (actorType == ActorType::PrimedTnt) {
                actorKnockbackPos = actor->mBuiltInComponents.get().mStateVectorComponent->mPos.get();
            } else {
                actorKnockbackPos = getEyePos(*actor);
            }

            Vec3  direction = actorKnockbackPos - mPos;
            float dirLen    = std::sqrt(direction.lengthSquared());
            if (dirLen >= 0.0001f) {
                direction.x *= 1.0f / dirLen;
                direction.y *= 1.0f / dirLen;
                direction.z *= 1.0f / dirLen;
            } else {
                direction = Vec3::ZERO();
            }

            float scaledForce = KnockbackRules::getScaledKnockbackForce(*actor, pow);
            Vec3  knockback   = direction * scaledForce;

            if (knockback.x != 0.0f || knockback.y != 0.0f || knockback.z != 0.0f) {
                if (actorType == ActorType::Player) {
                    if (source) {
                        auto* explodeComp = source->mEntityContext.get().mEnTTRegistry.try_get<ExplodeComponent>(
                            source->mEntityContext.get().mEntity
                        );
                        if (explodeComp && explodeComp->mNegatesFallDamage) {
                            auto& windComp =
                                actor->mEntityContext.get().mEnTTRegistry.get_or_emplace<WindChargeKnockbackComponent>(
                                    actor->mEntityContext.get().mEntity
                                );
                            float actorY = actor->mBuiltInComponents.get().mStateVectorComponent->mPos.get().y;
                            windComp.mKnockbackStartYCoordinate = actorY;
                            windComp.mTicksAfterAddition        = 0;
                        }
                    }
                }

                Vec3& posDelta  = actor->mBuiltInComponents.get().mStateVectorComponent->mPosDelta;
                posDelta       += knockback;

                ServerMovement::notifyOfServerInitiatedMotion(actor->mEntityContext);
            }
        }
    }

    mRadius = org;

    BlockPos minPos = BlockPos::MAX();
    BlockPos maxPos = BlockPos::MIN();

    if (mRadius > 0.0f) {
        ActorSoundIdentifier emptySoundId;
        level.broadcastSoundEvent(mRegion, mSoundExplosionType, mPos, -1, emptySoundId, false);
        level.broadcastLocalEvent(mRegion, mParticleType, mPos, static_cast<int>(mRadius));
    }

    mRegion.postGameEvent(source, GameEventRegistry::explode(), mPos, nullptr);

    int                          blockCount = 0;
    ParticlesBlockExplosionEvent event;
    event.mRadius = mRadius;
    event.mOrigin = mPos;

    BlockTypeRegistry vBlockTypeRegistry;
    auto const&       airBlock = vBlockTypeRegistry.getDefaultBlockState(BedrockBlockNames::Air(), true);

    bool forceFullDrops = false;
    if (source) {
        ActorType sourceType = source->getEntityTypeId();
        if (sourceType == ActorType::PrimedTnt || sourceType == ActorType::MinecartTNT) {
            auto const& gameRules = level.getGameRules();
            if (!gameRules.getBool(GameRuleId(37), false)) {
                forceFullDrops = true;
            }
        }
    }

    std::vector<std::pair<ItemStack, BlockPos>> mergedItemStacksToDrop;

    for (auto const& blockPos : mAffectedBlocks.get()) {
        auto const& block = mRegion.getBlock(blockPos);

        if (!level.isClientSide() && mCanToggleBlocks) {
            if (!source || source->getEntityTypeId() != ActorType::BreezeWindChargeProjectile) {
                block.mBlockType->_onHitByActivatingAttack(mRegion, blockPos, source);
            }
        }

        if (mBreaking) {
            if (!random.nextInt(8)) {
                Vec3 particlePos(
                    static_cast<float>(blockPos.x),
                    static_cast<float>(blockPos.y),
                    static_cast<float>(blockPos.z)
                );
                event.mPositions.get().emplace_back(particlePos);
            }

            if (!(block.mBlockType->mNameInfo.get().mFullName == BedrockBlockNames::Air())) {
                if (!level.isClientSide()) {
                    float dropRadius = forceFullDrops ? 1.0f : mRadius;

                    alignas(ResourceDropsContext) char rdc_buf[sizeof(ResourceDropsContext)]{};
                    auto& resourceDropsContext            = *reinterpret_cast<ResourceDropsContext*>(rdc_buf);
                    resourceDropsContext.mCause           = ResourceDropsCause::Explosion;
                    resourceDropsContext.mExplosionRadius = dropRadius;
                    resourceDropsContext.mUsedItem        = ItemStack::EMPTY_ITEM();
                    resourceDropsContext.mBlockPos        = blockPos;
                    resourceDropsContext.mDimensionType   = mRegion.getDimensionId();
                    resourceDropsContext.mBlockSource     = mRegion;

                    Randomize randomize(mRegion.getLevel().getRandom());

                    Explosion::_spawnExtraResourcesAndMergeItemDropsForBlock(
                        mRegion,
                        blockPos,
                        block,
                        randomize,
                        resourceDropsContext,
                        mergedItemStacksToDrop
                    );

                    auto const& extraBlock = mRegion.getExtraBlock(blockPos);
                    if (!(extraBlock.mBlockType->mNameInfo.get().mFullName == BedrockBlockNames::Air())) {
                        Explosion::_spawnExtraResourcesAndMergeItemDropsForBlock(
                            mRegion,
                            blockPos,
                            extraBlock,
                            randomize,
                            resourceDropsContext,
                            mergedItemStacksToDrop
                        );
                    }

                    block.mBlockType->onExploded(mRegion, blockPos, source);

                    mRegion.setExtraBlock(blockPos, airBlock, 3);

                    ActorChangeContext actorCtx{source};
                    BlockChangeContext changeCtx;
                    changeCtx.mContextSource = actorCtx;
                    mRegion.setBlock(blockPos, airBlock, 3, nullptr, changeCtx);
                }

                minPos.x = std::min(minPos.x, blockPos.x);
                minPos.y = std::min(minPos.y, blockPos.y);
                minPos.z = std::min(minPos.z, blockPos.z);
                maxPos.x = std::max(maxPos.x, blockPos.x);
                maxPos.y = std::max(maxPos.y, blockPos.y);
                maxPos.z = std::max(maxPos.z, blockPos.z);
                ++blockCount;
            }

            if (!level.isClientSide()) {
                auto& blockEventCoordinator = level.getBlockEventCoordinator();
                auto& dimension             = mRegion.getDimension();
                auto& coordListeners        = *reinterpret_cast<std::vector<BlockEventListener*>*>(
                    reinterpret_cast<char*>(&blockEventCoordinator) + 8
                );
                for (size_t li = 0; li < coordListeners.size(); ++li) {
                    coordListeners[li]->onBlockExploded(dimension, blockPos, block, source);
                }
            }
        }
    }

    for (auto& [itemStack, dropPos] : mergedItemStacksToDrop) {
        BlockType::popResource(mRegion, dropPos, itemStack);
    }

    if (!level.isClientSide()) {
        auto savedTag = event.save();

        LevelEventGenericPacketPayload payload;
        payload.mEventId = 2026;
        payload.mData.deepCopy(*savedTag);

        LevelEventGenericPacket packet(std::move(payload));

        BlockPos packetPos(mPos);
        mRegion.getDimension().sendPacketForPosition(packetPos, packet, nullptr);
    }

    if (mFire) {
        auto const& fireBlock = vBlockTypeRegistry.getDefaultBlockState(VanillaBlockTypeIds::Fire(), true);

        for (auto const& blockPos : mAffectedBlocks.get()) {
            auto const& block = mRegion.getBlock(blockPos);

            if (block.mBlockType->mNameInfo.get().mFullName == BedrockBlockNames::Air()) {
                BlockPos below(blockPos.x, blockPos.y - 1, blockPos.z);
                if (mRegion.getBlock(below).mBlockType->mMaterial.mSolid) {
                    if (!random.nextInt(3)) {
                        ActorChangeContext actorCtx{source};
                        BlockChangeContext changeCtx;
                        changeCtx.mContextSource = actorCtx;
                        mRegion.setBlock(blockPos, fireBlock, 3, nullptr, changeCtx);

                        minPos.x = std::min(minPos.x, blockPos.x);
                        minPos.y = std::min(minPos.y, blockPos.y);
                        minPos.z = std::min(minPos.z, blockPos.z);
                        maxPos.x = std::max(maxPos.x, blockPos.x);
                        maxPos.y = std::max(maxPos.y, blockPos.y);
                        maxPos.z = std::max(maxPos.z, blockPos.z);
                    }
                }
            }
        }
    }

    if (blockCount > 0) {
        BlockPos areaMin(minPos.x - 1, minPos.y - 1, minPos.z - 1);
        BlockPos areaMax(maxPos.x + 1, maxPos.y + 1, maxPos.z + 1);
        auto&    bsListeners = mRegion.mListeners.get();
        for (size_t li = 0; li < bsListeners.size(); ++li) {
            bsListeners[li]->onAreaChanged(mRegion, areaMin, areaMax);
        }
    }

    return true;
}
