#include "ila/event/minecraft/world/level/levelgen/structure/VillageFeatureEvent.h"
#include "ila/base/Gloabl.h"
#include "mc/world/level/Level.h"
#include <mc/world/level/ChunkPos.h>
#include <mc/world/level/dimension/OverworldDimension.h>
#include <mc/world/level/levelgen/WorldGenerator.h>
#include <mc/world/level/levelgen/structure/StructureFeatureRegistry.h>
#include <mc/world/level/levelgen/structure/VanillaStructureFeatureType.h>
#include <mc/world/level/levelgen/structure/VillageFeature.h>

namespace ila::mc::inline world::inline level::inline levelgen::inline structure {
void VillageFeatureConstructionEvent::serialize(CompoundTag& nbt) const {
    Event::serialize(nbt);
    std::vector<CompoundTagVariant> vctv = {};
    for (auto& ab : mAllowedBiomes) {
        vctv.push_back(ab.mValue);
    }
    nbt["allowedBiomes"]     = vctv;
    nbt["seed"]              = seed();
    nbt["townSpacing"]       = townSpacing();
    nbt["minTownSeparation"] = minTownSeparation();
}
void VillageFeatureConstructionEvent::deserialize(CompoundTag const& nbt) {
    Event::deserialize(nbt);
    mAllowedBiomes.clear();
    for (auto& ab : nbt["allowedBiomes"].get<ListTag>()) {
        mAllowedBiomes.push_back(BiomeIdType{static_cast<ushort>(ab.get<ShortTag>().data)});
    }
    seed()              = nbt["seed"];
    townSpacing()       = nbt["townSpacing"];
    minTownSeparation() = nbt["minTownSeparation"];
}
std::vector<BiomeIdType>& VillageFeatureConstructionEvent::allowedBiomes() const { return mAllowedBiomes; }
uint&                     VillageFeatureConstructionEvent::seed() const { return mSeed; }
int&                      VillageFeatureConstructionEvent::townSpacing() const { return mTownSpacing; }
int&                      VillageFeatureConstructionEvent::minTownSeparation() const { return mMinTownSeparation; }

LL_TYPE_INSTANCE_HOOK(
    OverworldDimensionCreateGeneratorHook,
    HookPriority::Normal,
    OverworldDimension,
    &OverworldDimension::$createGenerator,
    ::std::unique_ptr<::WorldGenerator>,
    ::br::worldgen::StructureSetRegistry const& pStructureSetRegistry
) {
    auto result         = origin(pStructureSetRegistry);
    auto villageFeature = static_cast<VillageFeature*>(
        result->mStructureFeatureRegistry->getStructureFeatureOfType(VanillaStructureFeatureType::Village())
    );
    auto seed = static_cast<Level&>(mLevel).getSeed();
    LLEventBus.publish(VillageFeatureConstructionEvent(
        villageFeature->mAllowedBiomes.get(),
        seed,
        villageFeature->mTownSpacing,
        villageFeature->mMinTownSeparation
    ));
    return result;
}

Event_Hook_Factory_Base(VillageFeatureConstruction, <OverworldDimensionCreateGeneratorHook>);

void CheckIfItIsAVillageGenerationChunkEvent::serialize(CompoundTag& nbt) const {
    Cancellable::serialize(nbt);
    nbt["preliminarySurfaceLevel"] = serializeRefObj(preliminarySurfaceLevel());
    nbt["biomeSource"]             = serializeRefObj(biomeSource());
    nbt["dimension"]               = serializeRefObj(dimension());
    nbt["chunkPos"]                = ListTag{chunkPos().x, chunkPos().y, chunkPos().z};
    nbt["random"]                  = serializeRefObj(random());
    nbt["levelSeed"]               = levelSeed();
}

IPreliminarySurfaceProvider const& CheckIfItIsAVillageGenerationChunkEvent::preliminarySurfaceLevel() const {
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
) {
    auto beforeEvent = CheckIfItIsAVillageGenerationChunkEvent(
        pPreliminarySurfaceLevel,
        pBiomeSource,
        pDimension,
        pChunkPos,
        pRandom,
        pLevelSeed
    );
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) {
        return false;
    }
    return origin(pBiomeSource, pRandom, pChunkPos, pLevelSeed, pPreliminarySurfaceLevel, pDimension);
}

Event_Hook_Factory_Base(CheckIfItIsAVillageGenerationChunk, <CheckIfItIsAVillageGenerationChunkHook>);
} // namespace ila::mc::inline world::inline level::inline levelgen::inline structure