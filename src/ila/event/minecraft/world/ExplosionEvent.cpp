#include "ila/event/minecraft/world/ExplosionEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/deps/core/math/Vec3.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/world/events/BlockEventCoordinator.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/Explosion.h>

namespace ila::mc::inline world
{

void ExplosionBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]            = ListTag { mExplosion.mPos->x, mExplosion.mPos->y, mExplosion.mPos->z };
    nbt["radius"]         = mExplosion.mRadius;
    nbt["affectedBlocks"] = ListTag {};
    for (auto& blockPos : *mExplosion.mAffectedBlocks)
    {
        nbt["affectedBlocks"].push_back(ListTag { blockPos.x, blockPos.y, blockPos.z });
    }
    nbt["fire"]                           = mExplosion.mFire;
    nbt["breaking"]                       = mExplosion.mBreaking;
    nbt["allowUnderwater"]                = mExplosion.mAllowUnderwater;
    nbt["canToggleBlocks"]                = mExplosion.mCanToggleBlocks;
    nbt["damageScaling"]                  = mExplosion.mDamageScaling;
    nbt["ignoreBlockExplosionResistance"] = mExplosion.mIgnoreBlockExplosionResistance;
    nbt["particleType"]                   = magic_enum::enum_name(mExplosion.mParticleType);
    nbt["soundExplosionType"]             = magic_enum::enum_name(mExplosion.mSoundExplosionType);
    nbt["sourceId"]                       = mExplosion.mSourceID->rawID;
    nbt["maxResistance"]                  = mExplosion.mMaxResistance;
    if (mExplosion.mInWaterOverride->has_value())
    {
        nbt["inWaterOverride"] = mExplosion.mInWaterOverride->value();
    }
    if (mExplosion.mTotalDamageOverride->has_value())
    {
        nbt["totalDamageOverride"] = mExplosion.mTotalDamageOverride->value();
    }
    nbt["knockbackScaling"] = mExplosion.mKnockbackScaling;
    nbt["dimId"]            = getDimensionName(blockSource());
}
void ExplosionBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mExplosion.mPos->x = nbt["pos"][0];
    mExplosion.mPos->y = nbt["pos"][1];
    mExplosion.mPos->z = nbt["pos"][2];
    mExplosion.mRadius = nbt["radius"];
    mExplosion.mAffectedBlocks->clear();
    for (auto& blockPos : nbt["affectedBlocks"].get<ListTag>())
    {
        mExplosion.mAffectedBlocks->insert(BlockPos(
            blockPos[0].get<IntTag>().data,
            blockPos[1].get<IntTag>().data,
            blockPos[2].get<IntTag>().data
        ));
    }
    mExplosion.mFire                           = nbt["fire"];
    mExplosion.mBreaking                       = nbt["breaking"];
    mExplosion.mAllowUnderwater                = nbt["allowUnderwater"];
    mExplosion.mCanToggleBlocks                = nbt["canToggleBlocks"];
    mExplosion.mDamageScaling                  = nbt["damageScaling"];
    mExplosion.mIgnoreBlockExplosionResistance = nbt["ignoreBlockExplosionResistance"];
    mExplosion.mParticleType =
        magic_enum::enum_cast<SharedTypes::Legacy::LevelEvent>(nbt["particleType"].get<StringTag>())
            .value_or(mExplosion.mParticleType);
    mExplosion.mSoundExplosionType =
        magic_enum::enum_cast<SharedTypes::Legacy::LevelSoundEvent>(nbt["soundExplosionType"].get<StringTag>()
        )
            .value_or(mExplosion.mSoundExplosionType);
    mExplosion.mSourceID->rawID = nbt["sourceId"];
    mExplosion.mMaxResistance   = nbt["maxResistance"];
    if (nbt.contains("inWaterOverride")) { mExplosion.mInWaterOverride = nbt["inWaterOverride"]; }
    else { mExplosion.mInWaterOverride->reset(); }
    if (nbt.contains("totalDamageOverride"))
    {
        mExplosion.mTotalDamageOverride = nbt["totalDamageOverride"];
    }
    else { mExplosion.mTotalDamageOverride->reset(); }
    mExplosion.mKnockbackScaling = nbt["knockbackScaling"];
}

void ExplosionAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]            = ListTag { mExplosion.mPos->x, mExplosion.mPos->y, mExplosion.mPos->z };
    nbt["radius"]         = mExplosion.mRadius;
    nbt["affectedBlocks"] = ListTag {};
    for (auto& blockPos : *mExplosion.mAffectedBlocks)
    {
        nbt["affectedBlocks"].push_back(ListTag { blockPos.x, blockPos.y, blockPos.z });
    }
    nbt["fire"]                           = mExplosion.mFire;
    nbt["breaking"]                       = mExplosion.mBreaking;
    nbt["allowUnderwater"]                = mExplosion.mAllowUnderwater;
    nbt["canToggleBlocks"]                = mExplosion.mCanToggleBlocks;
    nbt["damageScaling"]                  = mExplosion.mDamageScaling;
    nbt["ignoreBlockExplosionResistance"] = mExplosion.mIgnoreBlockExplosionResistance;
    nbt["particleType"]                   = magic_enum::enum_name(mExplosion.mParticleType);
    nbt["soundExplosionType"]             = magic_enum::enum_name(mExplosion.mSoundExplosionType);
    nbt["sourceId"]                       = mExplosion.mSourceID->rawID;
    nbt["maxResistance"]                  = mExplosion.mMaxResistance;
    if (mExplosion.mInWaterOverride->has_value())
    {
        nbt["inWaterOverride"] = mExplosion.mInWaterOverride->value();
    }
    if (mExplosion.mTotalDamageOverride->has_value())
    {
        nbt["totalDamageOverride"] = mExplosion.mTotalDamageOverride->value();
    }
    nbt["knockbackScaling"] = mExplosion.mKnockbackScaling;
    nbt["dimId"]            = getDimensionName(blockSource());
}

std::unique_ptr<ExplosionBeforeEvent> mBeforeEvent { nullptr };

LL_TYPE_INSTANCE_HOOK(
    ExplosionEventHook1,
    HookPriority::Normal,
    Explosion,
    &Explosion::explode,
    bool,
    IRandom& pRandom
)
{
    mBeforeEvent = std::make_unique<ExplosionBeforeEvent>(mRegion, *this);
    auto result  = origin(pRandom);
    if (result) { LLEventBus.publish(ExplosionAfterEvent(mRegion, *this)); }
    return result;
}

LL_TYPE_INSTANCE_HOOK(
    ExplosionEventHook2,
    HookPriority::Normal,
    BlockEventCoordinator,
    &BlockEventCoordinator::sendEvent,
    CoordinatorResult,
    EventRef<MutableBlockGameplayEvent<::CoordinatorResult>> pEvent
)
{
    return pEvent.get().visit([&]<typename T>(T& ev) {
        if constexpr (std::is_same_v<
                          std::remove_cvref_t<decltype(std::declval<T>().value())>,
                          ExplosionStartedEvent>)
        {
            std::swap(mBeforeEvent->mExplosion.mAffectedBlocks, ev.value().mBlocks);
            LLEventBus.publish(*mBeforeEvent);
            std::swap(mBeforeEvent->mExplosion.mAffectedBlocks, ev.value().mBlocks);
            if (mBeforeEvent->isCancelled()) return CoordinatorResult::Cancel;
        }
        return origin(pEvent);
    });
}

Event_Hook_Factory(Explosion, <ExplosionEventHook1, ExplosionEventHook2>);

} // namespace ila::mc::inline world