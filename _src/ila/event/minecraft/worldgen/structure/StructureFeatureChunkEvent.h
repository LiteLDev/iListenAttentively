#pragma once

#include "ila/base/Macro.h"
#include "ila/event/worldgen/structure/StructureEvent.h"
#include <ll/api/base/StdInt.h>
#include <ll/api/event/Cancellable.h>
#include <mc/deps/core/string/HashedString.h>
#include <mc/nbt/CompoundTag.h>

// clang-format off
class IPreliminarySurfaceProvider;
class BiomeSource;
class Dimension;
class ChunkPos;
class Random;
class StructureFeature;
// clang-format on

namespace ila::mc::inline worldgen::inline structure
{

class StructureFeatureChunkEvent final : public ll::event::Cancellable<StructureEvent>
{
public:
    StructureFeature&                  mFeature;
    HashedString const&                mFeatureIdentifier;
    IPreliminarySurfaceProvider const& mPreliminarySurfaceLevel;
    BiomeSource const&                 mBiomeSource;
    Dimension const&                   mDimension;
    ChunkPos const&                    mChunkPos;
    Random&                            mRandom;
    uint&                              mLevelSeed;

public:
    StructureFeatureChunkEvent(
        StructureFeature&                  feature,
        HashedString const&                featureIdentifier,
        IPreliminarySurfaceProvider const& preliminarySurfaceLevel,
        BiomeSource const&                 biomeSource,
        Dimension const&                   dimension,
        ChunkPos const&                    chunkPos,
        Random&                            random,
        uint&                              levelSeed
    );

    ILAPI void serialize(CompoundTag& nbt) const override;

};

} // namespace ila::mc::inline worldgen::inline structure