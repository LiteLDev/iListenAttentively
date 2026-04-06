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

void CheckIsStructureFeatureChunkEvent::serialize(CompoundTag& nbt) const {
    WorldEvent::serialize(nbt);
    nbt["feature"]                   = serializeRefObj(mFeature);
    nbt["preliminary_surface_level"] = serializeRefObj(mPreliminarySurfaceLevel);
    nbt["biome_source"]              = serializeRefObj(mBiomeSource);
    reflection::serialize_to(nbt["chunk_pos"], mChunkPos).value();
    nbt["random"] = serializeRefObj(mRandom);
    reflection::serialize_to(nbt["level_seed"], mLevelSeed).value();
}

template <std::derived_from<StructureFeature> T>
void CheckedIsStructureFeatureChunkEvent<T>::serialize(CompoundTag& nbt) const {
    CheckIsStructureFeatureChunkEvent::serialize(nbt);
    reflection::serialize_to(nbt["result"], mResult).value();
}

template <std::derived_from<StructureFeature> T>
void CheckedIsStructureFeatureChunkEvent<T>::deserialize(CompoundTag const& nbt) {
    CheckIsStructureFeatureChunkEvent::deserialize(nbt);
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
        if (                                                                                                           \
            eventPromise(                                                                                              \
                CheckingIsStructureFeatureChunkEvent{                                                                  \
                    **dimension.mBlockSource,                                                                          \
                    *this,                                                                                             \
                    preliminarySurfaceLevel,                                                                           \
                    biomeSource,                                                                                       \
                    chunkPos,                                                                                          \
                    random,                                                                                            \
                    levelSeed                                                                                          \
                }                                                                                                      \
            ).publish()                                                                                                \
            || eventPromise(                                                                                           \
                CheckingIsStructureFeatureChunkEvent<FeatureClass>{                                                    \
                    **dimension.mBlockSource,                                                                          \
                    *this,                                                                                             \
                    preliminarySurfaceLevel,                                                                           \
                    biomeSource,                                                                                       \
                    chunkPos,                                                                                          \
                    random,                                                                                            \
                    levelSeed                                                                                          \
                }                                                                                                      \
            ).publish()                                                                                                \
        ) {                                                                                                            \
            return false;                                                                                              \
        }                                                                                                              \
        auto result = origin(biomeSource, random, chunkPos, levelSeed, preliminarySurfaceLevel, dimension);            \
        eventPromise(                                                                                                  \
            CheckedIsStructureFeatureChunkEvent{                                                                       \
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
            CheckedIsStructureFeatureChunkEvent<FeatureClass>{                                                         \
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
        CheckingIs##FeatureClass##StructureFeatureChunkEventEmitter,                                                   \
        CheckingIsStructureFeatureChunkEvent<FeatureClass>,                                                            \
        <CheckIs##FeatureClass##StructureFeatureChunkEventHook>                                                        \
    );                                                                                                                 \
    EventHookFactory(                                                                                                  \
        CheckedIs##FeatureClass##StructureFeatureChunkEventEmitter,                                                    \
        CheckedIsStructureFeatureChunkEvent<FeatureClass>,                                                             \
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
    CheckingIsStructureFeatureChunkEventEmitter,
    CheckingIsStructureFeatureChunkEvent<>,
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
EventHookFactory(
    CheckedIsStructureFeatureChunkEventEmitter,
    CheckedIsStructureFeatureChunkEvent<>,
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