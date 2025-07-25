#include "ila/event/minecraft/world/level/levelgen/system/BiomeDecorationSystemEvent.h"

namespace ila::mc::inline world::inline level::inline levelgen::inline system
{
// BiomeDecorationSystemEvent
LevelChunk&        BiomeDecorationSystemEvent::levelChunk() const { return mLevelChunk; }
std::string const& BiomeDecorationSystemEvent::pass() const { return mPass; }
Random&            BiomeDecorationSystemEvent::random() const { return mRandom; }

// DecorateEvent
BlockSource&                       DecorateEvent::blockSource() const { return mBlockSource; }
std::vector<::Biome const*>&       DecorateEvent::uniqueBiomes() const { return mUniqueBiomes; }
IPreliminarySurfaceProvider const& DecorateEvent::preliminarySurfaceProvider() const
{
    return mPreliminarySurfaceProvider;
}

// DecorateBiomeEvent
gsl::span<::BiomeDecorationFeature const>& DecorateBiomeEvent::featureList() const { return mFeatureList; }
::Biome const*&                            DecorateBiomeEvent::biome() const { return mBiome; }
IPreliminarySurfaceProvider const&         DecorateBiomeEvent::preliminarySurfaceProvider() const
{
    return mPreliminarySurfaceProvider;
}

// DecorateLargeFeature1Event
GeneratorType&     DecorateLargeFeature1Event::generatorType() const { return mGeneratorType; }
uint const&        DecorateLargeFeature1Event::seed() const { return mSeed; }
BlockVolumeTarget& DecorateLargeFeature1Event::target() const { return mTarget; }
gsl::span<::BiomeDecorationFeature const>& DecorateLargeFeature1Event::featureList() const
{
    return mFeatureList;
}
ChunkPos const& DecorateLargeFeature1Event::chunkPos() const { return mChunkPos; }

// DecorateLargeFeature2Event
Biome const&       DecorateLargeFeature2Event::biome() const { return mBiome; }
BlockVolumeTarget& DecorateLargeFeature2Event::target() const { return mTarget; }
ChunkPos const&    DecorateLargeFeature2Event::chunkPos() const { return mChunkPos; }
} // namespace ila::mc::inline world::inline level::inline levelgen::inline system