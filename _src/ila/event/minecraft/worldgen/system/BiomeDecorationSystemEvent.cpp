#include "ila/event/worldgen/system/BiomeDecorationSystemEvent.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Memory.h"
#include "mc/world/level/ChunkPos.h"
#include <gsl/span>
#include <ila/base/Gloabl.h>
#include <ll/api/base/StdInt.h>
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/Event.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/util/molang/ExpressionNode.h>
#include <mc/world/level/BlockVolumeTarget.h>
#include <mc/world/level/GeneratorType.h>
#include <mc/world/level/IBlockWorldGenAPI.h>
#include <mc/world/level/WorldGenContext.h>
#include <mc/world/level/biome/components/BiomeDecorationFeature.h>
#include <mc/world/level/biome/systems/BiomeDecorationSystem.h>
#include <string>
#include <vector>

using namespace ila::mc;

//
void               BiomeDecorationSystemEvent::serialize(CompoundTag& nbt) const
{
    Event::serialize(nbt);
    nbt["levelChunk"] = serializeRefObj(mLevelChunk);
    nbt["pass"]       = mPass;
    nbt["random"]     = serializeRefObj(mRandom);
}

//
void DecorateEvent::serialize(CompoundTag& nbt) const
{
    ::ll::event::Cancellable<BiomeDecorationSystemEvent>::serialize(nbt);
    nbt["blockSource"]                = serializeRefObj(mBlockSource);
    nbt["uniqueBiomes"]               = serializeRefObj(mUniqueBiomes);
    nbt["preliminarySurfaceProvider"] = serializeRefObj(mPreliminarySurfaceProvider);
}

//
void         DecorateBiomeEvent::serialize(CompoundTag& nbt) const
{
    ::ll::event::Cancellable<BiomeDecorationSystemEvent>::serialize(nbt);
    nbt["featureList"]                = serializeRefObj(mFeatureList);
    nbt["biome"]                      = serializeRefObj(mBiome);
    nbt["preliminarySurfaceProvider"] = serializeRefObj(mPreliminarySurfaceProvider);
    nbt["blockSource"]                = serializeRefObj(mBlockSource);
}

//
void            DecorateLargeFeature1Event::serialize(CompoundTag& nbt) const
{
    ::ll::event::Cancellable<BiomeDecorationSystemEvent>::serialize(nbt);
    nbt["generatorType"] = serializeRefObj(mGeneratorType);
    nbt["seed"]          = mSeed;
    nbt["target"]        = serializeRefObj(mTarget);
    nbt["featureList"]   = serializeRefObj(mFeatureList);
    nbt["chunkPos"]      = ListTag { mChunkPos.x, mChunkPos.z };
}

//
void               DecorateLargeFeature2Event::serialize(CompoundTag& nbt) const
{
    ::ll::event::Cancellable<BiomeDecorationSystemEvent>::serialize(nbt);
    nbt["biome"]    = serializeRefObj(mBiome);
    nbt["target"]   = serializeRefObj(mTarget);
    nbt["chunkPos"] = ListTag { mChunkPos.x, mChunkPos.z };
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
