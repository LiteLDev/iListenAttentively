#include "ila/event/worldgen/CheckIsStructureFeatureChunkEvent.h"
#include "ila/base/Gloabl.i.h"
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/level/levelgen/structure/AncientCityFeature.h>
#include <mc/world/level/levelgen/structure/BastionFeature.h>
#include <mc/world/level/levelgen/structure/BuriedTreasureFeature.h>
#include <mc/world/level/levelgen/structure/EndCityFeature.h>
#include <mc/world/level/levelgen/structure/MineshaftFeature.h>
#include <mc/world/level/levelgen/structure/NetherFortressFeature.h>
#include <mc/world/level/levelgen/structure/OceanMonumentFeature.h>
#include <mc/world/level/levelgen/structure/OceanRuinFeature.h>
#include <mc/world/level/levelgen/structure/PillagerOutpostFeature.h>
#include <mc/world/level/levelgen/structure/RandomScatteredLargeFeature.h>
#include <mc/world/level/levelgen/structure/RuinedPortalFeature.h>
#include <mc/world/level/levelgen/structure/ShipwreckFeature.h>
#include <mc/world/level/levelgen/structure/StrongholdFeature.h>
#include <mc/world/level/levelgen/structure/StructureFeature.h>
#include <mc/world/level/levelgen/structure/VillageFeature.h>
#include <mc/world/level/levelgen/structure/WoodlandMansionFeature.h>

namespace ila::worldgen {

void ICheckIsStructureFeatureChunkEvent::serialize(CompoundTag& nbt) const {
    WorldEvent::serialize(nbt);
    nbt["feature"]                   = serializeRefObj(mFeature);
    nbt["preliminary_surface_level"] = serializeRefObj(mPreliminarySurfaceLevel);
    nbt["biome_source"]              = serializeRefObj(mBiomeSource);
    reflection::serialize_to(nbt["chunk_pos"], mChunkPos).value();
    nbt["random"] = serializeRefObj(mRandom);
    reflection::serialize_to(nbt["level_seed"], mLevelSeed).value();
    reflection::serialize_to(nbt["result"], mResult).value();
}

void ICheckIsStructureFeatureChunkEvent::deserialize(CompoundTag const& nbt) {
    WorldEvent::deserialize(nbt);
    reflection::deserialize(mResult, nbt["result"]).value();
}

// clang-format off
#define CheckIsStructureFeatureChunkHook(FeatureClass)                                                                 \
    LL_TYPE_INSTANCE_HOOK(                                                                                             \
        CheckIs##FeatureClass##StructureFeatureChunkEventHook,                                                         \
        HookPriority::Normal,                                                                                          \
        FeatureClass,                                                                                                  \
        &FeatureClass::$isFeatureChunk,                                                                                \
        bool,                                                                                                          \
        BiomeSource const&                 biomeSource,                                                                \
        Random&                            random,                                                                     \
        ChunkPos const&                    chunkPos,                                                                   \
        uint                               levelSeed,                                                                  \
        IPreliminarySurfaceProvider const& preliminarySurfaceLevel,                                                    \
        Dimension const&                   dimension                                                                   \
    ) {                                                                                                                \
        auto result = origin(biomeSource, random, chunkPos, levelSeed, preliminarySurfaceLevel, dimension);            \
        eventPromise(                                                                                                  \
            CheckIsStructureFeatureChunkEvent{                                                                         \
                **dimension.mBlockSource,                                                                              \
                *this,                                                                                                 \
                preliminarySurfaceLevel,                                                                               \
                biomeSource,                                                                                           \
                chunkPos,                                                                                              \
                random,                                                                                                \
                levelSeed,                                                                                             \
                result                                                                                                 \
            }                                                                                                          \
        ).onAfterEvent(                                                                                                \
            CheckIsStructureFeatureChunkEvent<FeatureClass>{                                                           \
                **dimension.mBlockSource,                                                                              \
                *this,                                                                                                 \
                preliminarySurfaceLevel,                                                                               \
                biomeSource,                                                                                           \
                chunkPos,                                                                                              \
                random,                                                                                                \
                levelSeed,                                                                                             \
                result                                                                                                 \
            }                                                                                                          \
        ).publish();                                                                                                   \
        return result;                                                                                                 \
    }                                                                                                                  \
    EventHookFactory(                                                                                                  \
        CheckIs##FeatureClass##StructureFeatureChunkEventEmitter,                                                      \
        CheckIsStructureFeatureChunkEvent<FeatureClass>,                                                               \
        <CheckIs##FeatureClass##StructureFeatureChunkEventHook>                                                        \
    );
// clang-format on

CheckIsStructureFeatureChunkHook(VillageFeature);
CheckIsStructureFeatureChunkHook(WoodlandMansionFeature);
CheckIsStructureFeatureChunkHook(AncientCityFeature);
CheckIsStructureFeatureChunkHook(BastionFeature);
CheckIsStructureFeatureChunkHook(BuriedTreasureFeature);
CheckIsStructureFeatureChunkHook(EndCityFeature);
CheckIsStructureFeatureChunkHook(MineshaftFeature);
CheckIsStructureFeatureChunkHook(NetherFortressFeature);
CheckIsStructureFeatureChunkHook(OceanMonumentFeature);
CheckIsStructureFeatureChunkHook(OceanRuinFeature);
CheckIsStructureFeatureChunkHook(PillagerOutpostFeature);
CheckIsStructureFeatureChunkHook(RandomScatteredLargeFeature);
CheckIsStructureFeatureChunkHook(RuinedPortalFeature);
CheckIsStructureFeatureChunkHook(ShipwreckFeature);
CheckIsStructureFeatureChunkHook(StrongholdFeature);

#undef CheckIsStructureFeatureChunkHook

// clang-format off
EventHookFactory(
    CheckIsStructureFeatureChunkEventEmitter,
    CheckIsStructureFeatureChunkEvent<>,
    <
        CheckIsVillageFeatureStructureFeatureChunkEventHook,
        CheckIsWoodlandMansionFeatureStructureFeatureChunkEventHook,
        CheckIsAncientCityFeatureStructureFeatureChunkEventHook,
        CheckIsBastionFeatureStructureFeatureChunkEventHook,
        CheckIsBuriedTreasureFeatureStructureFeatureChunkEventHook,
        CheckIsEndCityFeatureStructureFeatureChunkEventHook,
        CheckIsMineshaftFeatureStructureFeatureChunkEventHook,
        CheckIsNetherFortressFeatureStructureFeatureChunkEventHook,
        CheckIsOceanMonumentFeatureStructureFeatureChunkEventHook,
        CheckIsOceanRuinFeatureStructureFeatureChunkEventHook,
        CheckIsPillagerOutpostFeatureStructureFeatureChunkEventHook,
        CheckIsRandomScatteredLargeFeatureStructureFeatureChunkEventHook,
        CheckIsRuinedPortalFeatureStructureFeatureChunkEventHook,
        CheckIsShipwreckFeatureStructureFeatureChunkEventHook,
        CheckIsStrongholdFeatureStructureFeatureChunkEventHook
    >
)
// clang-format on

} // namespace ila::worldgen