#pragma once
#include "patch_mc/world/events/EventCoordinatorPimpl.i.h"
#include <mc/world/events/BlockEventListener.h>
#include <mc/world/events/EventCoordinator.h>

template <>
class EventCoordinator<BlockEventListener> : public EventCoordinatorPimpl<BlockEventListener> {};