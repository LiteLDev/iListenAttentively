#pragma once
#include "ila/base/Macro.h"
#include <ll/api/base/StdInt.h>
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/Event.h>
#include <mc/nbt/CompoundTag.h>

// clang-format off
class BiomeSource;
class ChunkPos;
class Dimension;
class HashedString;
class IPreliminarySurfaceProvider;
class Random;
class StructureFeature;
// clang-format on

namespace ila::worldgen {

enum class StructureFeatureKind {
    AncientCity,
    Bastion,
    BuriedTreasure,
    EndCity,
    Mineshaft,
    NetherFortress,
    OceanMonument,
    OceanRuin,
    PillagerOutpost,
    RandomScatteredLarge,
    RuinedPortal,
    Shipwreck,
    Stronghold,
    Village,
    WoodlandMansion,
};

class CheckIsStructureFeatureChunkEvent : public ll::event::Event {
private:
    StructureFeatureKind               mKind;
    StructureFeature&                  mFeature;
    HashedString const&                mFeatureIdentifier;
    IPreliminarySurfaceProvider const& mPreliminarySurfaceLevel;
    BiomeSource const&                 mBiomeSource;
    Dimension const&                   mDimension;
    ChunkPos const&                    mChunkPos;
    Random&                            mRandom;
    uint&                              mLevelSeed;

public:
    constexpr explicit CheckIsStructureFeatureChunkEvent(
        StructureFeatureKind               kind,
        StructureFeature&                  feature,
        HashedString const&                featureIdentifier,
        IPreliminarySurfaceProvider const& preliminarySurfaceLevel,
        BiomeSource const&                 biomeSource,
        Dimension const&                   dimension,
        ChunkPos const&                    chunkPos,
        Random&                            random,
        uint&                              levelSeed
    )
    : Event(),
      mKind(kind),
      mFeature(feature),
      mFeatureIdentifier(featureIdentifier),
      mPreliminarySurfaceLevel(preliminarySurfaceLevel),
      mBiomeSource(biomeSource),
      mDimension(dimension),
      mChunkPos(chunkPos),
      mRandom(random),
      mLevelSeed(levelSeed) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;

public:
    StructureFeatureKind               kind() const { return mKind; }
    StructureFeature&                  feature() const { return mFeature; }
    HashedString const&                featureIdentifier() const { return mFeatureIdentifier; }
    IPreliminarySurfaceProvider const& preliminarySurfaceLevel() const { return mPreliminarySurfaceLevel; }
    BiomeSource const&                 biomeSource() const { return mBiomeSource; }
    Dimension const&                   dimension() const { return mDimension; }
    ChunkPos const&                    chunkPos() const { return mChunkPos; }
    Random&                            random() const { return mRandom; }
    uint&                              levelSeed() const { return mLevelSeed; }
};

class CheckingIsStructureFeatureChunkEvent final : public ll::event::Cancellable<CheckIsStructureFeatureChunkEvent> {
public:
    using Cancellable::Cancellable;
};

class CheckedIsStructureFeatureChunkEvent final : public CheckIsStructureFeatureChunkEvent {
private:
    bool mResult;

public:
    constexpr explicit CheckedIsStructureFeatureChunkEvent(
        StructureFeatureKind               kind,
        StructureFeature&                  feature,
        HashedString const&                featureIdentifier,
        IPreliminarySurfaceProvider const& preliminarySurfaceLevel,
        BiomeSource const&                 biomeSource,
        Dimension const&                   dimension,
        ChunkPos const&                    chunkPos,
        Random&                            random,
        uint&                              levelSeed,
        bool                               result
    )
    : CheckIsStructureFeatureChunkEvent(
          kind,
          feature,
          featureIdentifier,
          preliminarySurfaceLevel,
          biomeSource,
          dimension,
          chunkPos,
          random,
          levelSeed
      ),
      mResult(result) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;

public:
    bool result() const { return mResult; }
};

#define ILA_DEFINE_CHECK_IS_STRUCTURE_FEATURE_CHUNK_EVENTS(FeatureKindName)                                            \
    class CheckIs##FeatureKindName##StructureFeatureChunkEvent : public CheckIsStructureFeatureChunkEvent {            \
    public:                                                                                                            \
        constexpr explicit CheckIs##FeatureKindName##StructureFeatureChunkEvent(                                       \
            StructureFeature&                  feature,                                                                \
            HashedString const&                featureIdentifier,                                                      \
            IPreliminarySurfaceProvider const& preliminarySurfaceLevel,                                                \
            BiomeSource const&                 biomeSource,                                                            \
            Dimension const&                   dimension,                                                              \
            ChunkPos const&                    chunkPos,                                                               \
            Random&                            random,                                                                 \
            uint&                              levelSeed                                                               \
        )                                                                                                              \
        : CheckIsStructureFeatureChunkEvent(                                                                           \
              StructureFeatureKind::FeatureKindName,                                                                   \
              feature,                                                                                                 \
              featureIdentifier,                                                                                       \
              preliminarySurfaceLevel,                                                                                 \
              biomeSource,                                                                                             \
              dimension,                                                                                               \
              chunkPos,                                                                                                \
              random,                                                                                                  \
              levelSeed                                                                                                \
          ) {}                                                                                                         \
    };                                                                                                                 \
                                                                                                                       \
    class CheckingIs##FeatureKindName##StructureFeatureChunkEvent final                                                \
    : public ll::event::Cancellable<CheckIs##FeatureKindName##StructureFeatureChunkEvent> {                            \
    public:                                                                                                            \
        using Cancellable::Cancellable;                                                                                \
    };                                                                                                                 \
                                                                                                                       \
    class CheckedIs##FeatureKindName##StructureFeatureChunkEvent final                                                 \
    : public CheckIs##FeatureKindName##StructureFeatureChunkEvent {                                                    \
    private:                                                                                                           \
        bool mResult;                                                                                                  \
                                                                                                                       \
    public:                                                                                                            \
        constexpr explicit CheckedIs##FeatureKindName##StructureFeatureChunkEvent(                                     \
            StructureFeature&                  feature,                                                                \
            HashedString const&                featureIdentifier,                                                      \
            IPreliminarySurfaceProvider const& preliminarySurfaceLevel,                                                \
            BiomeSource const&                 biomeSource,                                                            \
            Dimension const&                   dimension,                                                              \
            ChunkPos const&                    chunkPos,                                                               \
            Random&                            random,                                                                 \
            uint&                              levelSeed,                                                              \
            bool                               result                                                                  \
        )                                                                                                              \
        : CheckIs##FeatureKindName##StructureFeatureChunkEvent(                                                        \
              feature,                                                                                                 \
              featureIdentifier,                                                                                       \
              preliminarySurfaceLevel,                                                                                 \
              biomeSource,                                                                                             \
              dimension,                                                                                               \
              chunkPos,                                                                                                \
              random,                                                                                                  \
              levelSeed                                                                                                \
          ),                                                                                                           \
          mResult(result) {}                                                                                           \
                                                                                                                       \
    public:                                                                                                            \
        void serialize(CompoundTag& nbt) const override {                                                              \
            CheckIs##FeatureKindName##StructureFeatureChunkEvent::serialize(nbt);                                      \
            nbt["result"] = mResult;                                                                                   \
        }                                                                                                              \
                                                                                                                       \
        bool result() const { return mResult; }                                                                        \
    };

ILA_DEFINE_CHECK_IS_STRUCTURE_FEATURE_CHUNK_EVENTS(AncientCity)
ILA_DEFINE_CHECK_IS_STRUCTURE_FEATURE_CHUNK_EVENTS(Bastion)
ILA_DEFINE_CHECK_IS_STRUCTURE_FEATURE_CHUNK_EVENTS(BuriedTreasure)
ILA_DEFINE_CHECK_IS_STRUCTURE_FEATURE_CHUNK_EVENTS(EndCity)
ILA_DEFINE_CHECK_IS_STRUCTURE_FEATURE_CHUNK_EVENTS(Mineshaft)
ILA_DEFINE_CHECK_IS_STRUCTURE_FEATURE_CHUNK_EVENTS(NetherFortress)
ILA_DEFINE_CHECK_IS_STRUCTURE_FEATURE_CHUNK_EVENTS(OceanMonument)
ILA_DEFINE_CHECK_IS_STRUCTURE_FEATURE_CHUNK_EVENTS(OceanRuin)
ILA_DEFINE_CHECK_IS_STRUCTURE_FEATURE_CHUNK_EVENTS(PillagerOutpost)
ILA_DEFINE_CHECK_IS_STRUCTURE_FEATURE_CHUNK_EVENTS(RandomScatteredLarge)
ILA_DEFINE_CHECK_IS_STRUCTURE_FEATURE_CHUNK_EVENTS(RuinedPortal)
ILA_DEFINE_CHECK_IS_STRUCTURE_FEATURE_CHUNK_EVENTS(Shipwreck)
ILA_DEFINE_CHECK_IS_STRUCTURE_FEATURE_CHUNK_EVENTS(Stronghold)
ILA_DEFINE_CHECK_IS_STRUCTURE_FEATURE_CHUNK_EVENTS(Village)
ILA_DEFINE_CHECK_IS_STRUCTURE_FEATURE_CHUNK_EVENTS(WoodlandMansion)

#undef ILA_DEFINE_CHECK_IS_STRUCTURE_FEATURE_CHUNK_EVENTS

} // namespace ila::worldgen
