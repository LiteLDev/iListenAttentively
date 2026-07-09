#include "ila/event/world/WeatherUpdateEvent.h"
#include "ila/event/world/WeatherUpdateEvent.i.h"
#include "ila/utils/EventUtils.i.h"
#include <algorithm>
#include <ila/base/Gloabl.i.h>
#include <ll/api/chrono/GameChrono.h>
#include <ll/api/event/Event.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>
#include <mc/deps/nbt/CompoundTag.h>
#include <mc/world/level/IWeatherManagerProxy.h>
#include <mc/world/level/WeatherManager.h>

namespace ila::world {

void WeatherUpdateEvent::serialize(CompoundTag& nbt) const {
    Event::serialize(nbt);
    reflection::serialize_to(nbt["prev_type"], mPrevState.first).value();
    nbt["prev_duration"] = mPrevState.second.count();
    reflection::serialize_to(nbt["next_type"], mNextState.first).value();
    nbt["next_duration"] = mNextState.second.count();
}

void WeatherUpdateEvent::deserialize(CompoundTag const& nbt) {
    Event::deserialize(nbt);
    ll::reflection::deserialize(mNextState.first, nbt["next_type"]).value();
    mNextState.second = ll::chrono::ticks{nbt["next_duration"]};
    if (mNextState.first == WeatherUpdateEvent::Type::Clear) {
        mNextState.second = ll::chrono::ticks::zero();
    } else if (mNextState.second <= ll::chrono::ticks::zero()) {
        mNextState.first  = WeatherUpdateEvent::Type::Clear;
        mNextState.second = ll::chrono::ticks::zero();
    }
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

    // clang-format off
    auto beforeEvent = eventPromise(
        WeatherUpdatingEvent{
            {getTypeFromLevels(prevRainLevel, prevLightningLevel), prevTime},
            {getTypeFromLevels(nextRainLevel, nextLightningLevel), nextTime}
        }
    ).publish();
    // clang-format on
    if (beforeEvent) return;

    origin(
        getRainLevelFromType(*beforeEvent, WeatherUpdateEvent::Type::Rain),
        static_cast<int>(beforeEvent->nextDuration().count()),
        getRainLevelFromType(*beforeEvent, WeatherUpdateEvent::Type::Thunder),
        static_cast<int>(beforeEvent->nextDuration().count())
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

        // clang-format off
        eventPromise(WeatherUpdatedEvent{
            {getTypeFromLevels(prevRainLevel, prevLightningLevel), prevTime},
            {getTypeFromLevels(newRainLevel, newLightningLevel), newTime}
        }).publish();
        // clang-format on
    }
}

HookAliasImpl(WeatherUpdateEventHook, WeatherUpdateEventHook1);

} // namespace ila::world