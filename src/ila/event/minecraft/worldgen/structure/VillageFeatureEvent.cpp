#include "ila/event/minecraft/worldgen/structure/VillageFeatureEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/base/StdInt.h>
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/Event.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/memory/Hook.h>
#include <mc/_HeaderOutputPredefine.h>
#include <mc/common/BiomeIdType.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/CompoundTagVariant.h>
#include <mc/nbt/ListTag.h>
#include <mc/nbt/ShortTag.h>
#include <mc/world/level/ChunkPos.h>
#include <mc/world/level/levelgen/structure/VillageFeature.h>
#include <vector>

namespace ila::mc::inline worldgen::inline structure
{
void VillageFeatureConstructionEvent::serialize(CompoundTag& nbt) const
{
    Event::serialize(nbt);
    std::vector<CompoundTagVariant> vctv = {};
    for (auto& ab : mAllowedBiomes) { vctv.push_back(ab.mValue); }
    nbt["allowedBiomes"]     = vctv;
    nbt["seed"]              = mSeed;
    nbt["townSpacing"]       = mTownSpacing;
    nbt["minTownSeparation"] = mMinTownSeparation;
}
void VillageFeatureConstructionEvent::deserialize(CompoundTag const& nbt)
{
    Event::deserialize(nbt);
    mAllowedBiomes.clear();
    for (auto& ab : nbt["allowedBiomes"].get<ListTag>())
    {
        mAllowedBiomes.push_back(BiomeIdType { static_cast<ushort>(ab.get<ShortTag>().data) });
    }
    mSeed              = nbt["seed"];
    mTownSpacing       = nbt["townSpacing"];
    mMinTownSeparation = nbt["minTownSeparation"];
}

LL_TYPE_INSTANCE_HOOK(
    VillageFeatureConstructorHook,
    HookPriority::Normal,
    VillageFeature,
    &VillageFeature::$ctor,
    void*,
    uint pSeed,
    int  pTownSpacing,
    int  pMinTownSeparation
)
{
    // clang-format off
    LLEventBus.publish(VillageFeatureConstructionEvent(allowedBiomes, pSeed, pTownSpacing, pMinTownSeparation));
    // clang-format on
    return origin(pSeed, pTownSpacing, pMinTownSeparation);
}

Event_Hook_Factory_Base(VillageFeatureConstruction, <VillageFeatureConstructorHook>);

void CheckIfItIsAVillageGenerationChunkEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["preliminarySurfaceLevel"] = serializeRefObj(mPreliminarySurfaceLevel);
    nbt["biomeSource"]             = serializeRefObj(mBiomeSource);
    nbt["dimension"]               = serializeRefObj(mDimension);
    nbt["chunkPos"]                = ListTag { mChunkPos.x, mChunkPos.y, mChunkPos.z };
    nbt["random"]                  = serializeRefObj(mRandom);
    nbt["levelSeed"]               = mLevelSeed;
}

LL_TYPE_INSTANCE_HOOK(
    CheckIfItIsAVillageGenerationChunkHook,
    ll::memory::HookPriority::Normal,
    VillageFeature,
    &VillageFeature::$isFeatureChunk,
    bool,
    BiomeSource const&                 pBiomeSource,
    Random&                            pRandom,
    ChunkPos const&                    pChunkPos,
    uint                               pLevelSeed,
    IPreliminarySurfaceProvider const& pPreliminarySurfaceLevel,
    Dimension const&                   pDimension
)
{
    auto beforeEvent = CheckIfItIsAVillageGenerationChunkEvent(
        pPreliminarySurfaceLevel,
        pBiomeSource,
        pDimension,
        pChunkPos,
        pRandom,
        pLevelSeed
    );
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return false; }
    return origin(pBiomeSource, pRandom, pChunkPos, pLevelSeed, pPreliminarySurfaceLevel, pDimension);
}

Event_Hook_Factory_Base(CheckIfItIsAVillageGenerationChunk, <CheckIfItIsAVillageGenerationChunkHook>);
} // namespace ila::mc::inline worldgen::inline structure