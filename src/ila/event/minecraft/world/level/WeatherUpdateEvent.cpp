#include "ila/event/minecraft/world/level/WeatherUpdateEvent.h"
#include "ila/base/Gloabl.h"

namespace ila::mc::inline world::inline level {

void WeatherUpdateBeforeEvent::serialize(CompoundTag& nbt) const {
    Cancellable::serialize(nbt);
    nbt["rainLevel"]      = rainLevel();
    nbt["rainTime"]       = rainTime();
    nbt["lightningLevel"] = lightningLevel();
    nbt["lightningTime"]  = lightningTime();
}
void WeatherUpdateBeforeEvent::deserialize(CompoundTag const& nbt) {
    Cancellable::deserialize(nbt);
    rainLevel()      = nbt["rainLevel"];
    rainTime()       = nbt["rainTime"];
    lightningLevel() = nbt["lightningLevel"];
    lightningTime()  = nbt["lightningTime"];
}
float& WeatherUpdateBeforeEvent::rainLevel() const { return mRainLevel; };
int&   WeatherUpdateBeforeEvent::rainTime() const { return mRainTime; }
float& WeatherUpdateBeforeEvent::lightningLevel() const { return mLightningLevel; };
int&   WeatherUpdateBeforeEvent::lightningTime() const { return mLightningTime; };

void WeatherUpdateAfterEvent::serialize(CompoundTag& nbt) const {
    LevelEvent::serialize(nbt);
    nbt["rainLevel"]      = rainLevel();
    nbt["rainTime"]       = rainTime();
    nbt["lightningLevel"] = lightningLevel();
    nbt["lightningTime"]  = lightningTime();
}
float const& WeatherUpdateAfterEvent::rainLevel() const { return mRainLevel; };
int const&   WeatherUpdateAfterEvent::rainTime() const { return mRainTime; }
float const& WeatherUpdateAfterEvent::lightningLevel() const { return mLightningLevel; };
int const&   WeatherUpdateAfterEvent::lightningTime() const { return mLightningTime; };

LL_TYPE_INSTANCE_HOOK(
    WeatherUpdateEventHook,
    HookPriority::Normal,
    Level,
    &Level::$updateWeather,
    void,
    float pRainLevel,
    int   pRainTime,
    float pLightningLevel,
    int   pLightningTime
) {
    auto beforeEvent = WeatherUpdateBeforeEvent(*this, pRainLevel, pRainTime, pLightningLevel, pLightningTime);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) {
        return;
    }
    origin(pRainLevel, pRainTime, pLightningLevel, pLightningTime);
    LLEventBus.publish(WeatherUpdateAfterEvent(*this, pRainLevel, pRainTime, pLightningLevel, pLightningTime));
}

Event_Hook_Factory(WeatherUpdate, <WeatherUpdateEventHook>);

} // namespace ila::mc::inline world::inline level