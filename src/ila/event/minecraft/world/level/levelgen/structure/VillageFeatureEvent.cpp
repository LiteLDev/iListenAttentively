#include "ila/base/Gloabl.h"
#include <ila/event/minecraft/world/level/levelgen/structure/VillageFeatureEvent.h>
#include <mc/world/level/levelgen/structure/VillageFeature.h>

namespace ila::mc::inline world::inline level::inline levelgen::inline structure
{
void VillageFeatureConstructionEvent::serialize(CompoundTag& nbt) const
{
    Event::serialize(nbt);
    std::vector<CompoundTagVariant> vctv = {};
    for (auto& ab : this->mAllowedBiomes) { vctv.push_back(ab); }
    nbt["allowed_biomes"]      = vctv;
    nbt["seed"]                = this->mSeed;
    nbt["town_spacing"]        = this->mTownSpacing;
    nbt["min_town_separation"] = this->mMinTownSeparation;
}
::std::vector<uint64>& VillageFeatureConstructionEvent::allowedBiomes() const { return this->mAllowedBiomes; }
uint&                  VillageFeatureConstructionEvent::seed() const { return this->mSeed; }
int&                   VillageFeatureConstructionEvent::townSpacing() const { return this->mTownSpacing; }
int& VillageFeatureConstructionEvent::minTownSeparation() const { return this->mMinTownSeparation; }

LL_TYPE_INSTANCE_HOOK(
    VillageFeatureConstructorHook,
    ll::memory::HookPriority::Normal,
    VillageFeature,
    &VillageFeature::$ctor,
    void*,
    uint pSeed,
    int  pTownSpacing,
    int  pMinTownSeparation
)
{
    auto event =
        VillageFeatureConstructionEvent(this->allowedBiomes, pSeed, pTownSpacing, pMinTownSeparation);
    ::ll::event::EventBus::getInstance().publish(event);
    return origin(pSeed, pTownSpacing, pMinTownSeparation);
}

Event_Hook_Factory_Base(VillageFeatureConstruction, <VillageFeatureConstructorHook>);

void CheckIfItIsAVillageGenerationChunkEvent::serialize(CompoundTag& nbt) const
{
    ::ll::event::Cancellable<::ila::mc::StructureEvent>::serialize(nbt);
    nbt["preliminary_surface_level"] = serializeRefObj(this->mPreliminarySurfaceLevel);
    nbt["biome_source"]              = serializeRefObj(this->mBiomeSource);
    nbt["dimension"]                 = serializeRefObj(this->mDimension);
    nbt["chunk_pos"]["x"]            = this->mChunkPos.x;
    nbt["chunk_pos"]["y"]            = this->mChunkPos.y;
    nbt["chunk_pos"]["z"]            = this->mChunkPos.z;
    nbt["random"]                    = serializeRefObj(this->mRandom);
    nbt["level_seed"]                = this->mLevelSeed;
}

IPreliminarySurfaceProvider const& CheckIfItIsAVillageGenerationChunkEvent::preliminarySurfaceLevel() const
{
    return this->mPreliminarySurfaceLevel;
}
BiomeSource const& CheckIfItIsAVillageGenerationChunkEvent::biomeSource() const { return this->mBiomeSource; }
Dimension const&   CheckIfItIsAVillageGenerationChunkEvent::dimension() const { return this->mDimension; }
ChunkPos const&    CheckIfItIsAVillageGenerationChunkEvent::chunkPos() const { return this->mChunkPos; }
Random&            CheckIfItIsAVillageGenerationChunkEvent::random() const { return this->mRandom; }
uint&              CheckIfItIsAVillageGenerationChunkEvent::levelSeed() const { return this->mLevelSeed; }

LL_TYPE_INSTANCE_HOOK(
    CheckIfItIsAVillageGenerationChunkHook,
    ll::memory::HookPriority::Normal,
    VillageFeature,
    &VillageFeature::$isFeatureChunk,
    bool,
    ::BiomeSource const&                 pBiomeSource,
    ::Random&                            pRandom,
    ::ChunkPos const&                    pChunkPos,
    ::uint                               pLevelSeed,
    ::IPreliminarySurfaceProvider const& pPreliminarySurfaceLevel,
    ::Dimension const&                   pDimension
)
{
    auto event = CheckIfItIsAVillageGenerationChunkEvent(
        pPreliminarySurfaceLevel,
        pBiomeSource,
        pDimension,
        pChunkPos,
        pRandom,
        pLevelSeed
    );
    ::ll::event::EventBus::getInstance().publish(event);
    if (event.isCancelled()) { return false; }
    return origin(pBiomeSource, pRandom, pChunkPos, pLevelSeed, pPreliminarySurfaceLevel, pDimension);
}
} // namespace ila::mc::inline world::inline level::inline levelgen::inline structure