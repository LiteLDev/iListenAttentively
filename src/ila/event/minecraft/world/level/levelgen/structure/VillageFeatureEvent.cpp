#include "ila/event/minecraft/world/level/levelgen/structure/VillageFeatureEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/levelgen/structure/VillageFeature.h>

namespace ila::mc::inline world::inline level::inline levelgen::inline structure
{
void VillageFeatureConstructionEvent::serialize(CompoundTag& nbt) const
{
    Event::serialize(nbt);
    nbt["seed"]              = seed();
    nbt["townSpacing"]       = townSpacing();
    nbt["minTownSeparation"] = minTownSeparation();
}
void VillageFeatureConstructionEvent::deserialize(CompoundTag const& nbt)
{
    Event::deserialize(nbt);
    seed()              = nbt["seed"];
    townSpacing()       = nbt["townSpacing"];
    minTownSeparation() = nbt["minTownSeparation"];
}
uint& VillageFeatureConstructionEvent::seed() const { return mSeed; }
int&  VillageFeatureConstructionEvent::townSpacing() const { return mTownSpacing; }
int&  VillageFeatureConstructionEvent::minTownSeparation() const { return mMinTownSeparation; }

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
    LLEventBus.publish(VillageFeatureConstructionEvent(pSeed, pTownSpacing, pMinTownSeparation));
    return origin(pSeed, pTownSpacing, pMinTownSeparation);
}

Event_Hook_Factory_Base(VillageFeatureConstruction, <VillageFeatureConstructorHook>);

} // namespace ila::mc::inline world::inline level::inline levelgen::inline structure