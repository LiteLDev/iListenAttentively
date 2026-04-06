#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>
#include <mc/world/level/levelgen/structure/StructureFeature.h>

namespace ila::worldgen {

class CheckIsStructureFeatureChunkEvent : public ll::event::WorldEvent {
private:
    StructureFeature&                  mFeature;
    IPreliminarySurfaceProvider const& mPreliminarySurfaceLevel;
    BiomeSource const&                 mBiomeSource;
    ChunkPos const&                    mChunkPos;
    Random&                            mRandom;
    uint&                              mLevelSeed;

public:
    constexpr explicit CheckIsStructureFeatureChunkEvent(
        BlockSource&                       region,
        StructureFeature&                  feature,
        IPreliminarySurfaceProvider const& preliminarySurfaceLevel,
        BiomeSource const&                 biomeSource,
        ChunkPos const&                    chunkPos,
        Random&                            random,
        uint&                              levelSeed
    )
    : WorldEvent(region),
      mFeature(feature),
      mPreliminarySurfaceLevel(preliminarySurfaceLevel),
      mBiomeSource(biomeSource),
      mChunkPos(chunkPos),
      mRandom(random),
      mLevelSeed(levelSeed) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;

public:
    StructureFeature&                  feature() const { return mFeature; }
    HashedString const&                featureIdentifier() const { return feature().mStructureFeatureType; }
    IPreliminarySurfaceProvider const& preliminarySurfaceLevel() const { return mPreliminarySurfaceLevel; }
    BiomeSource const&                 biomeSource() const { return mBiomeSource; }
    ChunkPos const&                    chunkPos() const { return mChunkPos; }
    Random&                            random() const { return mRandom; }
    uint&                              levelSeed() const { return mLevelSeed; }
};

/** @warning This event is not available on the client side. */
template <std::derived_from<StructureFeature> T = StructureFeature>
class CheckingIsStructureFeatureChunkEvent final : public ll::event::Cancellable<CheckIsStructureFeatureChunkEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
template <std::derived_from<StructureFeature> T = StructureFeature>
class CheckedIsStructureFeatureChunkEvent final : public CheckIsStructureFeatureChunkEvent {
private:
    bool& mResult;

public:
    constexpr explicit CheckedIsStructureFeatureChunkEvent(
        BlockSource&                       region,
        StructureFeature&                  feature,
        IPreliminarySurfaceProvider const& preliminarySurfaceLevel,
        BiomeSource const&                 biomeSource,
        ChunkPos const&                    chunkPos,
        Random&                            random,
        uint&                              levelSeed,
        bool&                              result
    )
    : CheckIsStructureFeatureChunkEvent(
          region,
          feature,
          preliminarySurfaceLevel,
          biomeSource,
          chunkPos,
          random,
          levelSeed
      ),
      mResult(result) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

public:
    bool& result() const { return mResult; }
};

} // namespace ila::worldgen