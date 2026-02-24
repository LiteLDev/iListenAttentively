#include "ila/event/minecraft/world/WeatherUpdate.i.h"

namespace ila::mc::inline world {

EventHook(WeatherUpdatingEvent, WeatherUpdatedEvent, <WeatherUpdateEventHook1>);

} // namespace ila::mc::inline world