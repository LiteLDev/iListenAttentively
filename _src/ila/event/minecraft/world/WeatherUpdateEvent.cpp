#include "ila/event/minecraft/world/WeatherUpdateEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/LevelEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/level/Level.h>

namespace ila::mc::inline world
{

void WeatherUpdateBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["rainLevel"]      = mRainLevel;
    nbt["rainTime"]       = mRainTime;
    nbt["lightningLevel"] = mLightningLevel;
    nbt["lightningTime"]  = mLightningTime;
}
void WeatherUpdateBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mRainLevel      = nbt["rainLevel"];
    mRainTime       = nbt["rainTime"];
    mLightningLevel = nbt["lightningLevel"];
    mLightningTime  = nbt["lightningTime"];
};;;

void WeatherUpdateAfterEvent::serialize(CompoundTag& nbt) const
{
    LevelEvent::serialize(nbt);
    nbt["rainLevel"]      = mRainLevel;
    nbt["rainTime"]       = mRainTime;
    nbt["lightningLevel"] = mLightningLevel;
    nbt["lightningTime"]  = mLightningTime;
};;;

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
)
{
    auto beforeEvent =
        WeatherUpdateBeforeEvent(*this, pRainLevel, pRainTime, pLightningLevel, pLightningTime);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pRainLevel, pRainTime, pLightningLevel, pLightningTime);
    LLEventBus.publish(
        WeatherUpdateAfterEvent(*this, pRainLevel, pRainTime, pLightningLevel, pLightningTime)
    );
}

Event_Hook_Factory(WeatherUpdate, <WeatherUpdateEventHook>);

} // namespace ila::mc::inline world