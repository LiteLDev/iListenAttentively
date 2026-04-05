#include "ila/event/worldgen/CheckIsStructureFeatureChunkEvent.h"
#include "ila/base/Gloabl.i.h"
#include <ila/utils/EventUtils.i.h>
#include <ll/api/base/StdInt.h>
#include <ll/api/event/Event.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/memory/Hook.h>
#include <magic_enum.hpp>
#include <mc/_HeaderOutputPredefine.h>
#include <mc/deps/core/string/HashedString.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/level/ChunkPos.h>
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
#include <string>
#include <utility>

namespace ila::worldgen {

void CheckIsStructureFeatureChunkEvent::serialize(CompoundTag& nbt) const {
    Event::serialize(nbt);
    nbt["kind"]                      = std::string(magic_enum::enum_name(mKind));
    nbt["feature"]                   = serializeRefObj(mFeature);
    nbt["feature_identifier"]        = mFeatureIdentifier.getString();
    nbt["preliminary_surface_level"] = serializeRefObj(mPreliminarySurfaceLevel);
    nbt["biome_source"]              = serializeRefObj(mBiomeSource);
    nbt["dimension"]                 = serializeRefObj(mDimension);
    nbt["chunk_pos"]                 = CompoundTag{
                        {"x", mChunkPos.x},
                        {"z", mChunkPos.z}
    };
    nbt["random"]     = serializeRefObj(mRandom);
    nbt["level_seed"] = mLevelSeed;
}

void CheckedIsStructureFeatureChunkEvent::serialize(CompoundTag& nbt) const {
    CheckIsStructureFeatureChunkEvent::serialize(nbt);
    nbt["result"] = mResult;
}

namespace {

#define ILA_FOR_EACH_STRUCTURE_FEATURE(M)                                                                              \
    M(AncientCityFeature, AncientCity)                                                                                 \
    M(BastionFeature, Bastion)                                                                                         \
    M(BuriedTreasureFeature, BuriedTreasure)                                                                           \
    M(EndCityFeature, EndCity)                                                                                         \
    M(MineshaftFeature, Mineshaft)                                                                                     \
    M(NetherFortressFeature, NetherFortress)                                                                           \
    M(OceanMonumentFeature, OceanMonument)                                                                             \
    M(OceanRuinFeature, OceanRuin)                                                                                     \
    M(PillagerOutpostFeature, PillagerOutpost)                                                                         \
    M(RandomScatteredLargeFeature, RandomScatteredLarge)                                                               \
    M(RuinedPortalFeature, RuinedPortal)                                                                               \
    M(ShipwreckFeature, Shipwreck)                                                                                     \
    M(StrongholdFeature, Stronghold)                                                                                   \
    M(VillageFeature, Village)                                                                                         \
    M(WoodlandMansionFeature, WoodlandMansion)

#define ILA_DECLARE_HOOK_ALIAS(FeatureClass, FeatureKindName)                                                          \
    HookAliasDef(CheckIs##FeatureKindName##StructureFeatureChunkHookAlias);
ILA_FOR_EACH_STRUCTURE_FEATURE(ILA_DECLARE_HOOK_ALIAS)
#undef ILA_DECLARE_HOOK_ALIAS

template <typename SpecificCheckingEvent, typename SpecificCheckedEvent, typename FeatureType, typename OriginCall>
bool publishCheckIsStructureFeatureChunkEvents(
    StructureFeatureKind               kind,
    FeatureType&                       feature,
    OriginCall&&                       originCall,
    BiomeSource const&                 biomeSource,
    Random&                            random,
    ChunkPos const&                    chunkPos,
    uint&                              levelSeed,
    IPreliminarySurfaceProvider const& preliminarySurfaceLevel,
    Dimension const&                   dimension
) {
    auto genericEvent = CheckingIsStructureFeatureChunkEvent{
        kind,
        feature,
        feature.mStructureFeatureType,
        preliminarySurfaceLevel,
        biomeSource,
        dimension,
        chunkPos,
        random,
        levelSeed
    };
    auto specificEvent = SpecificCheckingEvent{
        feature,
        feature.mStructureFeatureType,
        preliminarySurfaceLevel,
        biomeSource,
        dimension,
        chunkPos,
        random,
        levelSeed
    };

    auto cancelled = static_cast<bool>(eventPromise(genericEvent).publish());
    cancelled      = static_cast<bool>(eventPromise(specificEvent).publish()) || cancelled;
    if (cancelled) {
        return false;
    }

    auto result = std::forward<
        OriginCall>(originCall)(feature, biomeSource, random, chunkPos, levelSeed, preliminarySurfaceLevel, dimension);

    eventPromise(
        CheckedIsStructureFeatureChunkEvent{
            kind,
            feature,
            feature.mStructureFeatureType,
            preliminarySurfaceLevel,
            biomeSource,
            dimension,
            chunkPos,
            random,
            levelSeed,
            result
        }
    )
        .publish();
    eventPromise(
        SpecificCheckedEvent{
            feature,
            feature.mStructureFeatureType,
            preliminarySurfaceLevel,
            biomeSource,
            dimension,
            chunkPos,
            random,
            levelSeed,
            result
        }
    )
        .publish();
    return result;
}

#define ILA_DEFINE_STRUCTURE_FEATURE_HOOK(FeatureClass, FeatureKindName)                                               \
    LL_TYPE_INSTANCE_HOOK(                                                                                             \
        CheckIs##FeatureKindName##StructureFeatureChunkHook,                                                           \
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
        return publishCheckIsStructureFeatureChunkEvents<                                                              \
            CheckingIs##FeatureKindName##StructureFeatureChunkEvent,                                                   \
            CheckedIs##FeatureKindName##StructureFeatureChunkEvent>(                                                   \
            StructureFeatureKind::FeatureKindName,                                                                     \
            *this,                                                                                                     \
            [&](FeatureClass&,                                                                                         \
                BiomeSource const&                 biomeSourceParam,                                                   \
                Random&                            randomParam,                                                        \
                ChunkPos const&                    chunkPosParam,                                                      \
                uint&                              levelSeedParam,                                                     \
                IPreliminarySurfaceProvider const& preliminarySurfaceLevelParam,                                       \
                Dimension const&                   dimensionParam) -> bool {                                                             \
            return origin(                                                                                             \
                biomeSourceParam,                                                                                      \
                randomParam,                                                                                           \
                chunkPosParam,                                                                                         \
                levelSeedParam,                                                                                        \
                preliminarySurfaceLevelParam,                                                                          \
                dimensionParam                                                                                         \
            );                                                                                                         \
        },                                                                                                             \
            biomeSource,                                                                                               \
            random,                                                                                                    \
            chunkPos,                                                                                                  \
            levelSeed,                                                                                                 \
            preliminarySurfaceLevel,                                                                                   \
            dimension                                                                                                  \
        );                                                                                                             \
    }

ILA_FOR_EACH_STRUCTURE_FEATURE(ILA_DEFINE_STRUCTURE_FEATURE_HOOK)
#undef ILA_DEFINE_STRUCTURE_FEATURE_HOOK

#define ILA_DEFINE_HOOK_ALIAS_IMPL(FeatureClass, FeatureKindName)                                                      \
    HookAliasImpl(                                                                                                     \
        CheckIs##FeatureKindName##StructureFeatureChunkHook,                                                           \
        CheckIs##FeatureKindName##StructureFeatureChunkHookAlias                                                       \
    );
ILA_FOR_EACH_STRUCTURE_FEATURE(ILA_DEFINE_HOOK_ALIAS_IMPL)
#undef ILA_DEFINE_HOOK_ALIAS_IMPL

#define ILA_STRUCTURE_FEATURE_CHUNK_HOOK_ALIASES                                                                       \
<                                                                                                                    \
        CheckIsAncientCityStructureFeatureChunkHookAlias,                                                                \
        CheckIsBastionStructureFeatureChunkHookAlias,                                                                    \
        CheckIsBuriedTreasureStructureFeatureChunkHookAlias,                                                             \
        CheckIsEndCityStructureFeatureChunkHookAlias,                                                                    \
        CheckIsMineshaftStructureFeatureChunkHookAlias,                                                                  \
        CheckIsNetherFortressStructureFeatureChunkHookAlias,                                                             \
        CheckIsOceanMonumentStructureFeatureChunkHookAlias,                                                              \
        CheckIsOceanRuinStructureFeatureChunkHookAlias,                                                                  \
        CheckIsPillagerOutpostStructureFeatureChunkHookAlias,                                                            \
        CheckIsRandomScatteredLargeStructureFeatureChunkHookAlias,                                                       \
        CheckIsRuinedPortalStructureFeatureChunkHookAlias,                                                               \
        CheckIsShipwreckStructureFeatureChunkHookAlias,                                                                  \
        CheckIsStrongholdStructureFeatureChunkHookAlias,                                                                 \
        CheckIsVillageStructureFeatureChunkHookAlias,                                                                    \
        CheckIsWoodlandMansionStructureFeatureChunkHookAlias                                                             \
    >

EventHook(
    CheckingIsStructureFeatureChunkEvent,
    CheckedIsStructureFeatureChunkEvent,
    ILA_STRUCTURE_FEATURE_CHUNK_HOOK_ALIASES
);

#define ILA_DEFINE_EVENT_HOOK(FeatureClass, FeatureKindName)                                                           \
    EventHook(                                                                                                         \
        CheckingIs##FeatureKindName##StructureFeatureChunkEvent,                                                       \
        CheckedIs##FeatureKindName##StructureFeatureChunkEvent,                                                        \
        ILA_STRUCTURE_FEATURE_CHUNK_HOOK_ALIASES                                                                       \
    );

ILA_FOR_EACH_STRUCTURE_FEATURE(ILA_DEFINE_EVENT_HOOK)
#undef ILA_DEFINE_EVENT_HOOK

#undef ILA_STRUCTURE_FEATURE_CHUNK_HOOK_ALIASES
#undef ILA_FOR_EACH_STRUCTURE_FEATURE

} // namespace

} // namespace ila::worldgen
