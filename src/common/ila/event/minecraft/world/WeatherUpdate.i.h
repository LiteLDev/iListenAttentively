#pragma once
#include "ila/base/Gloabl.i.h"
#include "ila/event/minecraft/world/WeatherUpdate.h"

namespace ila::mc::inline world {

WeatherUpdateEvent::Type getTypeFromLevels(float rainLevel, float lightningLevel);
float                    getRainLevelFromType(WeatherUpdatingEvent& event, WeatherUpdateEvent::Type type);

HookAliasDef(WeatherUpdateEventHook1);

} // namespace ila::mc::inline world