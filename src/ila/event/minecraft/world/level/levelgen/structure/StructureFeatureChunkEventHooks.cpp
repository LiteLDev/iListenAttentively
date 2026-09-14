#include "ila/base/Gloabl.h"
#include "ila/event/minecraft/world/level/levelgen/structure/StructureFeatureChunkEvent.h"
#include <fmt/format.h>
#include <ll/api/event/EmitterBase.h>
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
#include <mc/world/level/levelgen/structure/VillageFeature.h>
#include <mc/world/level/levelgen/structure/WoodlandMansionFeature.h>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace ila::mc::inline world::inline level::inline levelgen::inline structure {

static std::unique_ptr<ll::event::EmitterBase> StructureFeatureChunkEventEmitterFactory();

namespace {

std::unordered_set<std::string> gRegisteredIdentifiers{};
std::mutex                      gEmitterMutex;

void registerIdentifier(std::string_view identifier) {
    std::scoped_lock lock(gEmitterMutex);
    if (!gRegisteredIdentifiers.insert(std::string{identifier}).second) {
        return;
    }
    LLEventBus.setEventEmitter(
        StructureFeatureChunkEventEmitterFactory,
        ll::event::EventId{
            fmt::format("{0}<class {1}>", ll::reflection::type_name_v<StructureFeatureChunkEvent>, identifier)
        }
    );
}

template <class FeatureType, class OriginCall>
bool publishStructureFeatureChunkEvent(
    FeatureType&                       feature,
    OriginCall&&                       originCall,
    BiomeSource const&                 biomeSource,
    Random&                            random,
    ChunkPos const&                    chunkPos,
    uint                               levelSeed,
    IPreliminarySurfaceProvider const& preliminarySurfaceLevel,
    Dimension const&                   dimension
) {
    auto event = StructureFeatureChunkEvent(
        feature,
        feature.mStructureFeatureType,
        preliminarySurfaceLevel,
        biomeSource,
        dimension,
        chunkPos,
        random,
        levelSeed
    );
    auto& eventBus = LLEventBus;
    eventBus.publish(event);
    registerIdentifier(event.featureIdentifier().c_str());
    eventBus.publish(event, [&]() -> ll::event::EventId {
        return ll::event::EventId{fmt::format(
            "{0}<class {1}>",
            ll::reflection::type_name_v<StructureFeatureChunkEvent>,
            event.featureIdentifier().c_str()
        )};
    }());
    if (event.isCancelled()) {
        return false;
    }
    return std::forward<OriginCall>(originCall)(
        feature,
        biomeSource,
        random,
        chunkPos,
        event.levelSeed(),
        preliminarySurfaceLevel,
        dimension
    );
}

} // namespace

#define DEFINE_STRUCTURE_FEATURE_CHUNK_HOOK(FeatureClass)                                                              \
    LL_TYPE_INSTANCE_HOOK(                                                                                             \
        FeatureClass##ChunkEventHook,                                                                                  \
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
        return publishStructureFeatureChunkEvent(                                                                      \
            *this,                                                                                                     \
            [&](FeatureClass&                      featureParam,                                                       \
                BiomeSource const&                 biomeSourceParam,                                                   \
                Random&                            randomParam,                                                        \
                ChunkPos const&                    chunkPosParam,                                                      \
                uint                               levelSeedParam,                                                     \
                IPreliminarySurfaceProvider const& preliminarySurfaceParam,                                            \
                Dimension const&                   dimensionParam) -> bool {                                           \
                (void)featureParam;                                                                                    \
                return origin(                                                                                         \
                    biomeSourceParam,                                                                                  \
                    randomParam,                                                                                       \
                    chunkPosParam,                                                                                     \
                    levelSeedParam,                                                                                    \
                    preliminarySurfaceParam,                                                                           \
                    dimensionParam                                                                                     \
                );                                                                                                     \
            },                                                                                                         \
            biomeSource,                                                                                               \
            random,                                                                                                    \
            chunkPos,                                                                                                  \
            levelSeed,                                                                                                 \
            preliminarySurfaceLevel,                                                                                   \
            dimension                                                                                                  \
        );                                                                                                             \
    }

DEFINE_STRUCTURE_FEATURE_CHUNK_HOOK(AncientCityFeature);
DEFINE_STRUCTURE_FEATURE_CHUNK_HOOK(BastionFeature);
DEFINE_STRUCTURE_FEATURE_CHUNK_HOOK(BuriedTreasureFeature);
DEFINE_STRUCTURE_FEATURE_CHUNK_HOOK(EndCityFeature);
DEFINE_STRUCTURE_FEATURE_CHUNK_HOOK(MineshaftFeature);
DEFINE_STRUCTURE_FEATURE_CHUNK_HOOK(NetherFortressFeature);
DEFINE_STRUCTURE_FEATURE_CHUNK_HOOK(OceanMonumentFeature);
DEFINE_STRUCTURE_FEATURE_CHUNK_HOOK(OceanRuinFeature);
DEFINE_STRUCTURE_FEATURE_CHUNK_HOOK(PillagerOutpostFeature);
DEFINE_STRUCTURE_FEATURE_CHUNK_HOOK(RandomScatteredLargeFeature);
DEFINE_STRUCTURE_FEATURE_CHUNK_HOOK(RuinedPortalFeature);
DEFINE_STRUCTURE_FEATURE_CHUNK_HOOK(ShipwreckFeature);
DEFINE_STRUCTURE_FEATURE_CHUNK_HOOK(StrongholdFeature);
DEFINE_STRUCTURE_FEATURE_CHUNK_HOOK(VillageFeature);
DEFINE_STRUCTURE_FEATURE_CHUNK_HOOK(WoodlandMansionFeature);

#undef DEFINE_STRUCTURE_FEATURE_CHUNK_HOOK

class StructureFeatureChunkEventEmitter : public ll::event::Emitter<StructureFeatureChunkEventEmitterFactory> {
private:
    static inline bool reg = []() -> bool {
        LLEventBus.setEventEmitter(
            StructureFeatureChunkEventEmitterFactory,
            ll::event::EventId{ll::reflection::type_name_v<StructureFeatureChunkEvent>}
        );
        return true;
    }();

public:
    StructureFeatureChunkEventEmitter() {
        ll::memory::HookRegistrar<
            AncientCityFeatureChunkEventHook,
            BastionFeatureChunkEventHook,
            BuriedTreasureFeatureChunkEventHook,
            EndCityFeatureChunkEventHook,
            MineshaftFeatureChunkEventHook,
            NetherFortressFeatureChunkEventHook,
            OceanMonumentFeatureChunkEventHook,
            OceanRuinFeatureChunkEventHook,
            PillagerOutpostFeatureChunkEventHook,
            RandomScatteredLargeFeatureChunkEventHook,
            RuinedPortalFeatureChunkEventHook,
            ShipwreckFeatureChunkEventHook,
            StrongholdFeatureChunkEventHook,
            VillageFeatureChunkEventHook,
            WoodlandMansionFeatureChunkEventHook>()
            .hook();
    }

    ~StructureFeatureChunkEventEmitter() {
        ll::memory::HookRegistrar<
            AncientCityFeatureChunkEventHook,
            BastionFeatureChunkEventHook,
            BuriedTreasureFeatureChunkEventHook,
            EndCityFeatureChunkEventHook,
            MineshaftFeatureChunkEventHook,
            NetherFortressFeatureChunkEventHook,
            OceanMonumentFeatureChunkEventHook,
            OceanRuinFeatureChunkEventHook,
            PillagerOutpostFeatureChunkEventHook,
            RandomScatteredLargeFeatureChunkEventHook,
            RuinedPortalFeatureChunkEventHook,
            ShipwreckFeatureChunkEventHook,
            StrongholdFeatureChunkEventHook,
            VillageFeatureChunkEventHook,
            WoodlandMansionFeatureChunkEventHook>()
            .unhook();
    }
};

static std::unique_ptr<ll::event::EmitterBase> StructureFeatureChunkEventEmitterFactory() {
    return std::make_unique<StructureFeatureChunkEventEmitter>();
}

} // namespace ila::mc::inline world::inline level::inline levelgen::inline structure
