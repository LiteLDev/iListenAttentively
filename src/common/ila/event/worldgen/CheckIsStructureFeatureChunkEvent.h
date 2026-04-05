#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>
#include <mc/world/level/levelgen/structure/StructureFeature.h>

namespace ila::worldgen {

class ICheckIsStructureFeatureChunkEvent : public ll::event::WorldEvent {
private:
    StructureFeature&                  mFeature;
    IPreliminarySurfaceProvider const& mPreliminarySurfaceLevel;
    BiomeSource const&                 mBiomeSource;
    ChunkPos const&                    mChunkPos;
    Random&                            mRandom;
    uint&                              mLevelSeed;
    bool&                              mResult;

public:
    constexpr explicit ICheckIsStructureFeatureChunkEvent(
        BlockSource&                       region,
        StructureFeature&                  feature,
        IPreliminarySurfaceProvider const& preliminarySurfaceLevel,
        BiomeSource const&                 biomeSource,
        ChunkPos const&                    chunkPos,
        Random&                            random,
        uint&                              levelSeed,
        bool&                              result
    )
    : WorldEvent(region),
      mFeature(feature),
      mPreliminarySurfaceLevel(preliminarySurfaceLevel),
      mBiomeSource(biomeSource),
      mChunkPos(chunkPos),
      mRandom(random),
      mLevelSeed(levelSeed),
      mResult(result) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

public:
    StructureFeature&                  feature() const { return mFeature; }
    HashedString const&                featureIdentifier() const { return feature().mStructureFeatureType; }
    IPreliminarySurfaceProvider const& preliminarySurfaceLevel() const { return mPreliminarySurfaceLevel; }
    BiomeSource const&                 biomeSource() const { return mBiomeSource; }
    ChunkPos const&                    chunkPos() const { return mChunkPos; }
    Random&                            random() const { return mRandom; }
    uint&                              levelSeed() const { return mLevelSeed; }
    bool&                              result() const { return mResult; }
};

/** @warning This event is not available on the client side. */
template <std::derived_from<StructureFeature> T = StructureFeature>
class CheckIsStructureFeatureChunkEvent final : public ICheckIsStructureFeatureChunkEvent {
public:
    using ICheckIsStructureFeatureChunkEvent::ICheckIsStructureFeatureChunkEvent;
};

} // namespace ila::worldgen