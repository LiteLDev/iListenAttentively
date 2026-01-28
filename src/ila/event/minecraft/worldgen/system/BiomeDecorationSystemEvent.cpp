#include "ila/event/minecraft/worldgen/system/BiomeDecorationSystemEvent.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Memory.h"
#include "mc/world/level/ChunkPos.h"
#include <ila/base/Gloabl.h>

using namespace ila::mc;

// BiomeDecorationSystemEvent
LevelChunk&        BiomeDecorationSystemEvent::levelChunk() const { return mLevelChunk; }
std::string const& BiomeDecorationSystemEvent::pass() const { return mPass; }
Random&            BiomeDecorationSystemEvent::random() const { return mRandom; }
void               BiomeDecorationSystemEvent::serialize(CompoundTag& nbt) const
{
    Event::serialize(nbt);
    nbt["levelChunk"] = serializeRefObj(levelChunk());
    nbt["pass"]       = mPass;
    nbt["random"]     = serializeRefObj(random());
}

// DecorateEvent
BlockSource&                       DecorateEvent::blockSource() const { return mBlockSource; }
std::vector<::Biome const*>&       DecorateEvent::uniqueBiomes() const { return mUniqueBiomes; }
IPreliminarySurfaceProvider const& DecorateEvent::preliminarySurfaceProvider() const
{
    return mPreliminarySurfaceProvider;
}
void DecorateEvent::serialize(CompoundTag& nbt) const
{
    ::ll::event::Cancellable<BiomeDecorationSystemEvent>::serialize(nbt);
    nbt["blockSource"]                = serializeRefObj(blockSource());
    nbt["uniqueBiomes"]               = serializeRefObj(uniqueBiomes());
    nbt["preliminarySurfaceProvider"] = serializeRefObj(preliminarySurfaceProvider());
}

// DecorateBiomeEvent
gsl::span<::BiomeDecorationFeature const>& DecorateBiomeEvent::featureList() const { return mFeatureList; }
::Biome const*&                            DecorateBiomeEvent::biome() const { return mBiome; }
IPreliminarySurfaceProvider const&         DecorateBiomeEvent::preliminarySurfaceProvider() const
{
    return mPreliminarySurfaceProvider;
}
BlockSource& DecorateBiomeEvent::blockSource() const { return mBlockSource; }
void         DecorateBiomeEvent::serialize(CompoundTag& nbt) const
{
    ::ll::event::Cancellable<BiomeDecorationSystemEvent>::serialize(nbt);
    nbt["featureList"]                = serializeRefObj(featureList());
    nbt["biome"]                      = serializeRefObj(biome());
    nbt["preliminarySurfaceProvider"] = serializeRefObj(preliminarySurfaceProvider());
    nbt["blockSource"]                = serializeRefObj(blockSource());
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
void            DecorateLargeFeature1Event::serialize(CompoundTag& nbt) const
{
    ::ll::event::Cancellable<BiomeDecorationSystemEvent>::serialize(nbt);
    nbt["generatorType"] = serializeRefObj(generatorType());
    nbt["seed"]          = mSeed;
    nbt["target"]        = serializeRefObj(target());
    nbt["featureList"]   = serializeRefObj(featureList());
    nbt["chunkPos"]      = ListTag { chunkPos().x, chunkPos().z };
}

// DecorateLargeFeature2Event
Biome const&       DecorateLargeFeature2Event::biome() const { return mBiome; }
BlockVolumeTarget& DecorateLargeFeature2Event::target() const { return mTarget; }
ChunkPos const&    DecorateLargeFeature2Event::chunkPos() const { return mChunkPos; }
void               DecorateLargeFeature2Event::serialize(CompoundTag& nbt) const
{
    ::ll::event::Cancellable<BiomeDecorationSystemEvent>::serialize(nbt);
    nbt["biome"]    = serializeRefObj(biome());
    nbt["target"]   = serializeRefObj(target());
    nbt["chunkPos"] = ListTag { chunkPos().x, chunkPos().z };
}


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
    return origin(generatorType, seed, target, random, featureList, pos, pass);
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
