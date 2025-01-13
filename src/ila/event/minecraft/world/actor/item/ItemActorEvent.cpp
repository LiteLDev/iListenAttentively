#include "ila/event/minecraft/world/actor/item/ItemActorEvent.h"

namespace ila::event::inline actor::itemActor
{

ItemActor& ItemActorEvent::self() const { return static_cast<ItemActor&>(ActorEvent::self()); }

} // namespace ila::event::inline actor::itemActor