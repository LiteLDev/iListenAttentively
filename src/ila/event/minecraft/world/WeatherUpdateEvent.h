#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/LevelEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/level/Level.h>

namespace ila::mc::inline world
{
class WeatherUpdateBeforeEvent final : public ll::event::Cancellable<ll::event::LevelEvent>
{
public:
    float& mRainLevel;
    int&   mRainTime;
    float& mLightningLevel;
    int&   mLightningTime;

public:
    constexpr explicit WeatherUpdateBeforeEvent(
        Level& level,
        float& rainLevel,
        int&   rainTime,
        float& lightningLevel,
        int&   lightningTime
    )
        : Cancellable(level)
        , mRainLevel(rainLevel)
        , mRainTime(rainTime)
        , mLightningLevel(lightningLevel)
        , mLightningTime(lightningTime)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class WeatherUpdateAfterEvent final : public ll::event::LevelEvent
{
public:
    float const& mRainLevel;
    int const&   mRainTime;
    float const& mLightningLevel;
    int const&   mLightningTime;

public:
    constexpr explicit WeatherUpdateAfterEvent(
        Level&       level,
        float const& rainLevel,
        int const&   rainTime,
        float const& lightningLevel,
        int const&   lightningTime
    )
        : LevelEvent(level)
        , mRainLevel(rainLevel)
        , mRainTime(rainTime)
        , mLightningLevel(lightningLevel)
        , mLightningTime(lightningTime)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline world