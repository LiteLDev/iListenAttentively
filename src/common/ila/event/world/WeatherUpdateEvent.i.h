#pragma once
#include "ila/base/Gloabl.i.h"
#include "ila/event/world/WeatherUpdateEvent.h"

namespace ila::world {

WeatherUpdateEvent::Type getTypeFromLevels(float rainLevel, float lightningLevel);
float                    getRainLevelFromType(WeatherUpdatingEvent& event, WeatherUpdateEvent::Type type);

HookAliasDef(WeatherUpdateEventHook1);

} // namespace ila::world