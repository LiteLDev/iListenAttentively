#include "ila/event/world/WeatherUpdateEvent.i.h"

namespace ila::world {

EventHook(WeatherUpdatingEvent, WeatherUpdatedEvent, <WeatherUpdateEventHook1>);

} // namespace ila::world