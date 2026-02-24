#include "ila/event/minecraft/world/WeatherUpdate.h"
#include "ila/base/Gloabl.i.h"
#include <mc/deps/core/math/Vec3.h>
#include <mc/world/events/LevelEventCoordinator.h>
#include <mc/world/level/IWeatherManagerProxy.h>
#include <mc/world/level/WeatherManager.h>

namespace ila::mc::inline world {

LL_TYPE_INSTANCE_HOOK(
    WeatherUpdateEventHook1,
    HookPriority::Normal,
    WeatherManager,
    &WeatherManager::updateWeather,
    void,
    float nextRainLevel,
    int   nextRainTime,
    float nextLightningLevel,
    int   nextLightningTime
) {
    static constexpr auto getState = [](float rainLevel, float lightningLevel) {
        if (lightningLevel > 0.0f) return WeatherUpdateEvent::Type::Thunder;
        if (rainLevel > 0.0f) return WeatherUpdateEvent::Type::Rain;
        return WeatherUpdateEvent::Type::Clear;
    };
    static constexpr auto getLevel = [](WeatherUpdatingEvent& event, WeatherUpdateEvent::Type type) {
        switch (event.nextType()) {
        case WeatherUpdateEvent::Type::Clear:
            return 0.0f;
        case WeatherUpdateEvent::Type::Rain:
            return static_cast<float>(type == WeatherUpdateEvent::Type::Rain);
        case WeatherUpdateEvent::Type::Thunder:
            return static_cast<float>(
                type == WeatherUpdateEvent::Type::Rain || type == WeatherUpdateEvent::Type::Thunder
            );
        default:
            return 0.0f;
        }
    };

    auto nextTime = ll::chrono::ticks{nextRainTime > nextLightningTime ? nextRainTime : nextLightningTime};

    auto prevRainLevel      = mWeatherManagerProxy->getRainLevel();
    auto prevLightningLevel = mWeatherManagerProxy->getLightningLevel();
    auto prevRainTime       = mWeatherManagerProxy->getRainTime();
    auto prevLightningTime  = mWeatherManagerProxy->getLightningTime();
    auto prevTime           = ll::chrono::ticks{prevRainTime > prevLightningTime ? prevRainTime : prevLightningTime};

    auto beforeEvent = WeatherUpdatingEvent(
        {getState(prevRainLevel, prevLightningLevel), prevTime},
        {getState(nextRainLevel, nextLightningLevel), nextTime}
    );
    getLLEventBus().publish(beforeEvent);
    if (beforeEvent.isCancelled()) return;
    origin(
        getLevel(beforeEvent, WeatherUpdateEvent::Type::Rain),
        static_cast<int>(beforeEvent.nextDuration().count()),
        getLevel(beforeEvent, WeatherUpdateEvent::Type::Thunder),
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
            {getState(prevRainLevel, prevLightningLevel), prevTime},
            {getState(newRainLevel, newLightningLevel), newTime}
        );
        getLLEventBus().publish(afterEvent);
    }
}

// LL_TYPE_INSTANCE_HOOK(
//     WeatherUpdateHook2,
//     HookPriority::Normal,
//     LevelEventCoordinator,
//     &LevelEventCoordinator::sendEvent,
//     CoordinatorResult,
//     EventRef<MutableLevelGameplayEvent<CoordinatorResult>> event
// ) {
// }

EventHook(WeatherUpdatingEvent, WeatherUpdatedEvent, <WeatherUpdateEventHook1>);

}

// WeatherUpdateHook2: type=3002, pos=(0, 0, 0), data=65535
// WeatherUpdateHook2: type=3004, pos=(0, 0, 0), data=0
// WeatherUpdateHook2: type=3001, pos=(0, 0, 0), data=65535
// WeatherUpdateHook2: type=3004, pos=(0, 0, 0), data=0
// WeatherUpdateHook2: type=3003, pos=(0, 0, 0), data=0
// WeatherUpdateHook2: type=3001, pos=(0, 0, 0), data=65535
// WeatherUpdateHook2: type=3002, pos=(0, 0, 0), data=65535