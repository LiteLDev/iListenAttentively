#include "ila/event/minecraft/worldgen/structure/StructureFeatureChunkEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/base/StdInt.h>
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <mc/_HeaderOutputPredefine.h>
#include <mc/deps/core/string/HashedString.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/world/level/ChunkPos.h>
#include <mc/world/level/levelgen/structure/StructureFeature.h>

namespace ila::mc::inline worldgen::inline structure
{

StructureFeatureChunkEvent::StructureFeatureChunkEvent(
    StructureFeature&                  feature,
    HashedString const&                featureIdentifier,
    IPreliminarySurfaceProvider const& preliminarySurfaceLevel,
    BiomeSource const&                 biomeSource,
    Dimension const&                   dimension,
    ChunkPos const&                    chunkPos,
    Random&                            random,
    uint&                              levelSeed
)
    : Cancellable()
    , mFeature(feature)
    , mFeatureIdentifier(featureIdentifier)
    , mPreliminarySurfaceLevel(preliminarySurfaceLevel)
    , mBiomeSource(biomeSource)
    , mDimension(dimension)
    , mChunkPos(chunkPos)
    , mRandom(random)
    , mLevelSeed(levelSeed)
{
}

void StructureFeatureChunkEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["structureFeature"]        = serializeRefObj(structureFeature());
    nbt["featureIdentifier"]       = featureIdentifier().getString();
    nbt["preliminarySurfaceLevel"] = serializeRefObj(preliminarySurfaceLevel());
    nbt["biomeSource"]             = serializeRefObj(biomeSource());
    nbt["dimension"]               = serializeRefObj(dimension());
    nbt["chunkPos"]                = ListTag { chunkPos().x, chunkPos().y, chunkPos().z };
    nbt["random"]                  = serializeRefObj(random());
    nbt["levelSeed"]               = levelSeed();
}

StructureFeature& StructureFeatureChunkEvent::structureFeature() const { return mFeature; }

HashedString const& StructureFeatureChunkEvent::featureIdentifier() const { return mFeatureIdentifier; }

IPreliminarySurfaceProvider const& StructureFeatureChunkEvent::preliminarySurfaceLevel() const
{
    return mPreliminarySurfaceLevel;
}

BiomeSource const& StructureFeatureChunkEvent::biomeSource() const { return mBiomeSource; }

Dimension const& StructureFeatureChunkEvent::dimension() const { return mDimension; }

ChunkPos const& StructureFeatureChunkEvent::chunkPos() const { return mChunkPos; }

Random& StructureFeatureChunkEvent::random() const { return mRandom; }

uint& StructureFeatureChunkEvent::levelSeed() const { return mLevelSeed; }

} // namespace ila::mc::inline worldgen::inline structure
