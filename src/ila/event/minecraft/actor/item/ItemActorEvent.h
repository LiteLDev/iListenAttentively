#include "ila/base/Macro.h"
#include <ll/api/event/entity/ActorEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/item/ItemActor.h>

namespace ila::mc::inline actor::inline item
{

class ItemActorEvent : public ll::event::ActorEvent
{
public:
    constexpr explicit ItemActorEvent(ItemActor& actor)
        : ActorEvent(actor)
    {
    }

    void       serialize(CompoundTag&) const override;
    ItemActor& self() const { return static_cast<ItemActor&>(ActorEvent::self()); }
};

} // namespace ila::mc::inline actor::inline item
