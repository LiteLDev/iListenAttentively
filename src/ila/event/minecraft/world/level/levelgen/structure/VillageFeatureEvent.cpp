#include "ila/base/Gloabl.h"
#include <ila/event/minecraft/world/level/levelgen/structure/VillageFeatureEvent.h>
#include <mc/world/level/levelgen/structure/VillageFeature.h>

namespace ila::mc::inline world::inline level::inline levelgen::inline structure
{
void VillageFeatureConstructionEvent::serialize(CompoundTag& nbt) const
{
    Event::serialize(nbt);
    std::vector<CompoundTagVariant> vctv = {};
    for (auto& ab : this->mAllowedBiomes) { vctv.push_back(ab); }
    nbt["allowed_biomes"]      = vctv;
    nbt["seed"]                = this->mSeed;
    nbt["town_spacing"]        = this->mTownSpacing;
    nbt["min_town_separation"] = this->mMinTownSeparation;
}
::std::vector<uint64>& VillageFeatureConstructionEvent::allowedBiomes() const { return this->mAllowedBiomes; }
uint&                  VillageFeatureConstructionEvent::seed() const { return this->mSeed; }
int&                   VillageFeatureConstructionEvent::townSpacing() const { return this->mTownSpacing; }
int& VillageFeatureConstructionEvent::minTownSeparation() const { return this->mMinTownSeparation; }

LL_TYPE_INSTANCE_HOOK(
    VillageFeatureConstructorHook,
    ll::memory::HookPriority::Normal,
    VillageFeature,
    &VillageFeature::$ctor,
    void*,
    uint pSeed,
    int  pTownSpacing,
    int  pMinTownSeparation
)
{
    auto event =
        VillageFeatureConstructionEvent(this->allowedBiomes, pSeed, pTownSpacing, pMinTownSeparation);
    ::ll::event::EventBus::getInstance().publish(event);
    return origin(pSeed, pTownSpacing, pMinTownSeparation);
}

Event_Hook_Factory_Base(VillageFeatureConstruction, <VillageFeatureConstructorHook>);
;
} // namespace ila::mc::inline world::inline level::inline levelgen::inline structure