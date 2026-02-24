#include "ila/event/minecraft/world/WeatherUpdate.i.h"
#include "ila/base/Gloabl.i.h"
#include <ll/api/chrono/GameChrono.h>
#include <ll/api/reflection/Deserialization.h>
#include <magic_enum.hpp>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/level/IWeatherManagerProxy.h>
#include <mc/world/level/WeatherManager.h>

namespace ila::mc::inline world {

void WeatherUpdateEvent::serialize(CompoundTag& nbt) const {
    nbt["prev_type"]     = magic_enum::enum_name(mPrevState.first);
    nbt["prev_duration"] = mPrevState.second.count();
    nbt["next_type"]     = magic_enum::enum_name(mNextState.first);
    nbt["next_duration"] = mNextState.second.count();
}

void WeatherUpdatingEvent::deserialize(CompoundTag const& nbt) {
    mPrevState.first  = ll::reflection::deserialize_to<Type>(nbt["prev_type"]).value();
    mPrevState.second = ll::chrono::ticks{nbt["prev_duration"]};
    mNextState.first  = ll::reflection::deserialize_to<Type>(nbt["next_type"]).value();
    mNextState.second = ll::chrono::ticks{nbt["next_duration"]};
}

WeatherUpdateEvent::Type getTypeFromLevels(float rainLevel, float lightningLevel) {
    if (lightningLevel > 0.0f) return WeatherUpdateEvent::Type::Thunder;
    if (rainLevel > 0.0f) return WeatherUpdateEvent::Type::Rain;
    return WeatherUpdateEvent::Type::Clear;
};

float getRainLevelFromType(WeatherUpdatingEvent& event, WeatherUpdateEvent::Type type) {
    switch (event.nextType()) {
    case WeatherUpdateEvent::Type::Clear:
        return 0.0f;
    case WeatherUpdateEvent::Type::Rain:
        return static_cast<float>(type == WeatherUpdateEvent::Type::Rain);
    case WeatherUpdateEvent::Type::Thunder:
        return static_cast<float>(type == WeatherUpdateEvent::Type::Rain || type == WeatherUpdateEvent::Type::Thunder);
    default:
        return 0.0f;
    }
};

LL_TYPE_INSTANCE_HOOK(
    WeatherUpdateEventHook,
    HookPriority::Normal,
    WeatherManager,
    &WeatherManager::updateWeather,
    void,
    float nextRainLevel,
    int   nextRainTime,
    float nextLightningLevel,
    int   nextLightningTime
) {
    auto nextTime = ll::chrono::ticks{std::max(nextRainTime, nextLightningTime)};

    auto prevRainLevel      = mWeatherManagerProxy->getRainLevel();
    auto prevLightningLevel = mWeatherManagerProxy->getLightningLevel();
    auto prevRainTime       = mWeatherManagerProxy->getRainTime();
    auto prevLightningTime  = mWeatherManagerProxy->getLightningTime();
    auto prevTime           = ll::chrono::ticks{std::max(prevRainTime, prevLightningTime)};

    auto beforeEvent = WeatherUpdatingEvent(
        {getTypeFromLevels(prevRainLevel, prevLightningLevel), prevTime},
        {getTypeFromLevels(nextRainLevel, nextLightningLevel), nextTime}
    );
    getLLEventBus().publish(beforeEvent);
    if (beforeEvent.isCancelled()) return;
    origin(
        getRainLevelFromType(beforeEvent, WeatherUpdateEvent::Type::Rain),
        static_cast<int>(beforeEvent.nextDuration().count()),
        getRainLevelFromType(beforeEvent, WeatherUpdateEvent::Type::Thunder),
        static_cast<int>(beforeEvent.nextDuration().count())
    );

    {
        auto newRainLevel      = mWeatherManagerProxy->getRainLevel();
        auto newLightningLevel = mWeatherManagerProxy->getLightningLevel();
        auto newRainTime       = mWeatherManagerProxy->getRainTime();
        auto newLightningTime  = mWeatherManagerProxy->getLightningTime();
        auto newTime           = ll::chrono::ticks{newRainTime > newLightningTime ? newRainTime : newLightningTime};

        if (prevRainLevel == newRainLevel && prevRainTime == newRainTime && prevLightningLevel == newLightningLevel
            && prevLightningTime == newLightningTime) {
            return;
        }

        auto afterEvent = WeatherUpdatedEvent(
            {getTypeFromLevels(prevRainLevel, prevLightningLevel), prevTime},
            {getTypeFromLevels(newRainLevel, newLightningLevel), newTime}
        );
        getLLEventBus().publish(afterEvent);
    }
}

HookAliasImpl(WeatherUpdateEventHook, WeatherUpdateEventHook1);

} // namespace ila::mc::inline world