#include "ila/event/minecraft/world/level/levelgen/system/BiomeDecorationSystemEvent.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Memory.h"
#include <ila/base/Gloabl.h>

using namespace ila::mc;

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
BlockSource& DecorateBiomeEvent::blockSource() const { return mBlockSource; }

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


LL_STATIC_HOOK(
    DecorateEventHook,
    ll::memory::HookPriority::Normal,
    &BiomeDecorationSystem::decorate,
    void,
    ::LevelChunk&                        lc,
    ::BlockSource&                       source,
    ::Random&                            random,
    ::std::vector<::Biome const*>&       uniqueBiomes,
    ::std::string const&                 pass,
    ::IPreliminarySurfaceProvider const& preliminarySurfaceProvider
)
{
    auto event = DecorateEvent(lc, source, random, uniqueBiomes, pass, preliminarySurfaceProvider);
    LLEventBus.publish(event);
    if (event.isCancelled()) { return; }
    origin(lc, source, random, uniqueBiomes, pass, preliminarySurfaceProvider);
}
LL_STATIC_HOOK(
    DecorateBiomeEventHook,
    ll::memory::HookPriority::Normal,
    &BiomeDecorationSystem::decorateBiome,
    bool,
    ::LevelChunk&                               lc,
    ::BlockSource&                              source,
    ::Random&                                   random,
    ::gsl::span<::BiomeDecorationFeature const> featureList,
    ::std::string const&                        pass,
    ::Biome const*                              biome,
    ::IPreliminarySurfaceProvider const&        preliminarySurfaceProvider
)
{
    auto event = DecorateBiomeEvent(lc, source, random, featureList, pass, biome, preliminarySurfaceProvider);
    LLEventBus.publish(event);
    if (event.isCancelled()) { return false; }
    return origin(lc, source, random, featureList, pass, biome, preliminarySurfaceProvider);
}

LL_STATIC_HOOK(
    DecorateLargeFeature1EventHook,
    ll::memory::HookPriority::Normal,
    &BiomeDecorationSystem::decorateLargeFeature,
    bool,
    ::GeneratorType                             generatorType,
    uint const&                                 seed,
    ::BlockVolumeTarget&                        target,
    ::Random&                                   random,
    ::gsl::span<::BiomeDecorationFeature const> featureList,
    ::ChunkPos const&                           pos,
    ::std::string const&                        pass
)
{
    auto event = DecorateLargeFeature1Event(generatorType, seed, target, random, featureList, pos, pass);
    LLEventBus.publish(event);
    if (event.isCancelled()) { return false; }
    origin(generatorType, seed, target, random, featureList, pos, pass);
}

LL_STATIC_HOOK(
    DecorateLargeFeature2EventHook,
    ll::memory::HookPriority::Normal,
    &BiomeDecorationSystem::decorateLargeFeature,
    void,
    ::Biome const&       biome,
    ::LevelChunk&        lc,
    ::BlockVolumeTarget& target,
    ::Random&            random,
    ::ChunkPos const&    pos,
    ::std::string const& pass
)
{
    auto event = DecorateLargeFeature2Event(biome, lc, target, random, pos, pass);
    LLEventBus.publish(event);
    if (event.isCancelled()) { return; }
    origin(biome, lc, target, random, pos, pass);
}
