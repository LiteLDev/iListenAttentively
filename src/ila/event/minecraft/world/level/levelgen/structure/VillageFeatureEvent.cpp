#include "ila/event/minecraft/world/level/levelgen/structure/VillageFeatureEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/levelgen/structure/VillageFeature.h>

namespace ila::mc::inline world::inline level::inline levelgen::inline structure
{
void VillageFeatureConstructionEvent::serialize(CompoundTag& nbt) const
{
    Event::serialize(nbt);
    std::vector<CompoundTagVariant> vctv = {};
    for (auto& ab : mAllowedBiomes) { vctv.push_back(ab); }
    nbt["allowedBiomes"]     = vctv;
    nbt["seed"]              = seed();
    nbt["townSpacing"]       = townSpacing();
    nbt["minTownSeparation"] = minTownSeparation();
}
void VillageFeatureConstructionEvent::deserialize(CompoundTag const& nbt)
{
    Event::deserialize(nbt);
    mAllowedBiomes.clear();
    for (auto& ab : nbt["allowedBiomes"].get<ListTag>()) { mAllowedBiomes.push_back(ab); }
    seed()              = nbt["seed"];
    townSpacing()       = nbt["townSpacing"];
    minTownSeparation() = nbt["minTownSeparation"];
}
std::vector<uint64>& VillageFeatureConstructionEvent::allowedBiomes() const { return mAllowedBiomes; }
uint&                VillageFeatureConstructionEvent::seed() const { return mSeed; }
int&                 VillageFeatureConstructionEvent::townSpacing() const { return mTownSpacing; }
int&                 VillageFeatureConstructionEvent::minTownSeparation() const { return mMinTownSeparation; }

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
    nbt["preliminarySurfaceLevel"] = serializeRefObj(preliminarySurfaceLevel());
    nbt["biomeSource"]             = serializeRefObj(biomeSource());
    nbt["dimension"]               = serializeRefObj(dimension());
    nbt["chunkPos"]                = ListTag { chunkPos().x, chunkPos().y, chunkPos().z };
    nbt["random"]                  = serializeRefObj(random());
    nbt["levelSeed"]               = levelSeed();
}

IPreliminarySurfaceProvider const& CheckIfItIsAVillageGenerationChunkEvent::preliminarySurfaceLevel() const
{
    return mPreliminarySurfaceLevel;
}
BiomeSource const& CheckIfItIsAVillageGenerationChunkEvent::biomeSource() const { return mBiomeSource; }
Dimension const&   CheckIfItIsAVillageGenerationChunkEvent::dimension() const { return mDimension; }
ChunkPos const&    CheckIfItIsAVillageGenerationChunkEvent::chunkPos() const { return mChunkPos; }
Random&            CheckIfItIsAVillageGenerationChunkEvent::random() const { return mRandom; }
uint&              CheckIfItIsAVillageGenerationChunkEvent::levelSeed() const { return mLevelSeed; }

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
} // namespace ila::mc::inline world::inline level::inline levelgen::inline structure