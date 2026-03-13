#include "ila/event/worldgen/structure/StructureFeatureChunkEvent.h"
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
    nbt["structureFeature"]        = serializeRefObj(mFeature);
    nbt["featureIdentifier"]       = mFeatureIdentifier.getString();
    nbt["preliminarySurfaceLevel"] = serializeRefObj(mPreliminarySurfaceLevel);
    nbt["biomeSource"]             = serializeRefObj(mBiomeSource);
    nbt["dimension"]               = serializeRefObj(mDimension);
    nbt["chunkPos"]                = ListTag { mChunkPos.x, mChunkPos.y, mChunkPos.z };
    nbt["random"]                  = serializeRefObj(mRandom);
    nbt["levelSeed"]               = mLevelSeed;
}

} // namespace ila::mc::inline worldgen::inline structure
