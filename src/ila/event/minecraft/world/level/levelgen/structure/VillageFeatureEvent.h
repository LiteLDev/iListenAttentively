#pragma once
#include "ila/base/Macro.h"
#include "ila/event/minecraft/world/level/levelgen/structure/StructureEvent.h"
#include <ll/api/event/Event.h>

namespace ila::mc::inline world::inline level::inline levelgen::inline structure
{

class VillageFeatureConstructionEvent final : public ::ila::mc::StructureEvent
{
protected:
    uint& mSeed;
    int&  mTownSpacing;
    int&  mMinTownSeparation;

public:
    constexpr explicit VillageFeatureConstructionEvent(
        uint& pSeed,
        int&  pTownSpacing,
        int&  pMinTownSeparation
    )
        : StructureEvent()
        , mSeed(pSeed)
        , mTownSpacing(pTownSpacing)
        , mMinTownSeparation(pMinTownSeparation)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

public:
    ILNDAPI uint& seed() const;
    ILNDAPI int&  townSpacing() const;
    ILNDAPI int&  minTownSeparation() const;
};

} // namespace ila::mc::inline world::inline level::inline levelgen::inline structure