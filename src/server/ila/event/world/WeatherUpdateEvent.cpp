#include "ila/event/world/WeatherUpdateEvent.i.h"

namespace ila::mc::inline world {

EventHook(WeatherUpdatingEvent, WeatherUpdatedEvent, <WeatherUpdateEventHook1>);

} // namespace ila::mc::inline world