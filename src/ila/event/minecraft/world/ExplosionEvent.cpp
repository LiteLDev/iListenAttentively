#include "ila/event/minecraft/world/ExplosionEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/deps/core/math/Vec3.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/Explosion.h>

using namespace ila::mc;

void ExplosionBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]            = ListTag { explosion().mPos->x, explosion().mPos->y, explosion().mPos->z };
    nbt["radius"]         = explosion().mRadius;
    nbt["affectedBlocks"] = ListTag {};
    for (auto& blockPos : *explosion().mAffectedBlocks)
    {
        nbt["affectedBlocks"].push_back(ListTag { blockPos.x, blockPos.y, blockPos.z });
    }
    nbt["fire"]                           = explosion().mFire;
    nbt["breaking"]                       = explosion().mBreaking;
    nbt["allowUnderwater"]                = explosion().mAllowUnderwater;
    nbt["canToggleBlocks"]                = explosion().mCanToggleBlocks;
    nbt["damageScaling"]                  = explosion().mDamageScaling;
    nbt["ignoreBlockExplosionResistance"] = explosion().mIgnoreBlockExplosionResistance;
    nbt["particleType"]                   = magic_enum::enum_name(explosion().mParticleType);
    nbt["soundExplosionType"]             = magic_enum::enum_name(explosion().mSoundExplosionType);
    nbt["sourceId"]                       = explosion().mSourceID->rawID;
    nbt["maxResistance"]                  = explosion().mMaxResistance;
    if (explosion().mInWaterOverride->has_value())
    {
        nbt["inWaterOverride"] = explosion().mInWaterOverride->value();
    }
    if (explosion().mTotalDamageOverride->has_value())
    {
        nbt["totalDamageOverride"] = explosion().mTotalDamageOverride->value();
    }
    nbt["knockbackScaling"] = explosion().mKnockbackScaling;
    nbt["dimId"]            = getDimensionName(blockSource());
}
void ExplosionBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    explosion().mPos->x = nbt["pos"][0];
    explosion().mPos->y = nbt["pos"][1];
    explosion().mPos->z = nbt["pos"][2];
    explosion().mRadius = nbt["radius"];
    explosion().mAffectedBlocks->clear();
    for (auto& blockPos : nbt["affectedBlocks"].get<ListTag>())
    {
        explosion().mAffectedBlocks->insert(BlockPos(
            blockPos[0].get<IntTag>().data,
            blockPos[1].get<IntTag>().data,
            blockPos[2].get<IntTag>().data
        ));
    }
    explosion().mFire                           = nbt["fire"];
    explosion().mBreaking                       = nbt["breaking"];
    explosion().mAllowUnderwater                = nbt["allowUnderwater"];
    explosion().mCanToggleBlocks                = nbt["canToggleBlocks"];
    explosion().mDamageScaling                  = nbt["damageScaling"];
    explosion().mIgnoreBlockExplosionResistance = nbt["ignoreBlockExplosionResistance"];
    explosion().mParticleType =
        magic_enum::enum_cast<SharedTypes::Legacy::LevelEvent>(nbt["particleType"].get<StringTag>())
            .value_or(explosion().mParticleType);
    explosion().mSoundExplosionType =
        magic_enum::enum_cast<SharedTypes::Legacy::LevelSoundEvent>(nbt["soundExplosionType"].get<StringTag>()
        )
            .value_or(explosion().mSoundExplosionType);
    explosion().mSourceID->rawID = nbt["sourceId"];
    explosion().mMaxResistance   = nbt["maxResistance"];
    if (nbt.contains("inWaterOverride")) { explosion().mInWaterOverride = nbt["inWaterOverride"]; }
    else { explosion().mInWaterOverride->reset(); }
    if (nbt.contains("totalDamageOverride"))
    {
        explosion().mTotalDamageOverride = nbt["totalDamageOverride"];
    }
    else { explosion().mTotalDamageOverride->reset(); }
    explosion().mKnockbackScaling = nbt["knockbackScaling"];
}
Explosion& ExplosionBeforeEvent::explosion() const { return mExplosion; }

void ExplosionAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]            = ListTag { explosion().mPos->x, explosion().mPos->y, explosion().mPos->z };
    nbt["radius"]         = explosion().mRadius;
    nbt["affectedBlocks"] = ListTag {};
    for (auto& blockPos : *explosion().mAffectedBlocks)
    {
        nbt["affectedBlocks"].push_back(ListTag { blockPos.x, blockPos.y, blockPos.z });
    }
    nbt["fire"]                           = explosion().mFire;
    nbt["breaking"]                       = explosion().mBreaking;
    nbt["allowUnderwater"]                = explosion().mAllowUnderwater;
    nbt["canToggleBlocks"]                = explosion().mCanToggleBlocks;
    nbt["damageScaling"]                  = explosion().mDamageScaling;
    nbt["ignoreBlockExplosionResistance"] = explosion().mIgnoreBlockExplosionResistance;
    nbt["particleType"]                   = magic_enum::enum_name(explosion().mParticleType);
    nbt["soundExplosionType"]             = magic_enum::enum_name(explosion().mSoundExplosionType);
    nbt["sourceId"]                       = explosion().mSourceID->rawID;
    nbt["maxResistance"]                  = explosion().mMaxResistance;
    if (explosion().mInWaterOverride->has_value())
    {
        nbt["inWaterOverride"] = explosion().mInWaterOverride->value();
    }
    if (explosion().mTotalDamageOverride->has_value())
    {
        nbt["totalDamageOverride"] = explosion().mTotalDamageOverride->value();
    }
    nbt["knockbackScaling"] = explosion().mKnockbackScaling;
    nbt["dimId"]            = getDimensionName(blockSource());
}
Explosion const& ExplosionAfterEvent::explosion() const { return mExplosion; }

LL_TYPE_INSTANCE_HOOK(ExplosionEventHook, HookPriority::Normal, Explosion, &Explosion::explode, bool)
{
    auto beforeEvent = ExplosionBeforeEvent(mRegion, *this);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return false; }
    auto result = origin();
    if (result) { LLEventBus.publish(ExplosionAfterEvent(mRegion, *this)); }
    return result;
}

Event_Hook_Factory(Explosion, <ExplosionEventHook>);
