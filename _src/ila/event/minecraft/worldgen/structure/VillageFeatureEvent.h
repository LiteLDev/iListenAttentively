#pragma once
#include "ila/base/Macro.h"
#include "ila/event/minecraft/worldgen/structure/StructureEvent.h"
#include <ll/api/base/StdInt.h>
#include <ll/api/event/Cancellable.h>
#include <mc/common/BiomeIdType.h>
#include <mc/nbt/CompoundTag.h>
#include <vector>

// clang-format off
class IPreliminarySurfaceProvider;
class BiomeSource;
class Dimension;
class ChunkPos;
class Random;
// clang-format on

namespace ila::mc::inline worldgen::inline structure
{

class VillageFeatureConstructionEvent final : public ila::mc::StructureEvent
{
public:
    std::vector<BiomeIdType>& mAllowedBiomes;

    uint& mSeed;
    int&  mTownSpacing;
    int&  mMinTownSeparation;

public:
    constexpr explicit VillageFeatureConstructionEvent(
        std::vector<BiomeIdType>& pAllowedBiomes,
        uint&                     pSeed,
        int&                      pTownSpacing,
        int&                      pMinTownSeparation
    )
        : StructureEvent()
        , mAllowedBiomes(pAllowedBiomes)
        , mSeed(pSeed)
        , mTownSpacing(pTownSpacing)
        , mMinTownSeparation(pMinTownSeparation)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

public:
};

class CheckIfItIsAVillageGenerationChunkEvent final : public ll::event::Cancellable<ila::mc::StructureEvent>
{
public:
    IPreliminarySurfaceProvider const& mPreliminarySurfaceLevel;
    BiomeSource const&                 mBiomeSource;
    Dimension const&                   mDimension;
    ChunkPos const&                    mChunkPos;
    Random&                            mRandom;
    uint&                              mLevelSeed;

public:
    constexpr explicit CheckIfItIsAVillageGenerationChunkEvent(
        IPreliminarySurfaceProvider const& pPreliminarySurfaceLevel,
        BiomeSource const&                 pBiomeSource,
        Dimension const&                   pDimension,
        ChunkPos const&                    pChunkPos,
        Random&                            pRandom,
        uint&                              pLevelSeed
    )
        : Cancellable()
        , mPreliminarySurfaceLevel(pPreliminarySurfaceLevel)
        , mBiomeSource(pBiomeSource)
        , mDimension(pDimension)
        , mChunkPos(pChunkPos)
        , mRandom(pRandom)
        , mLevelSeed(pLevelSeed)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

public:
};

} // namespace ila::mc::inline worldgen::inline structure