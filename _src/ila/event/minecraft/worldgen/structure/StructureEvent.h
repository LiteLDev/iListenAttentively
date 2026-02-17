#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Event.h>
#include <ll/api/event/world/WorldEvent.h>

namespace ila::mc::inline worldgen::inline structure
{

class StructureEvent : public ll::event::Event
{
public:
    constexpr explicit StructureEvent()
        : Event()
    {
    }
};

} // namespace ila::mc::inline worldgen::inline structure