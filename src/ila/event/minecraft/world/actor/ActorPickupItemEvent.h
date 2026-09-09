#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/entity/MobEvent.h>

// clang-format off
class ItemActor;
// clang-format on

namespace ila::mc::inline world::inline actor
{
class ActorPickupItemBeforeEvent final : public ll::event::Cancellable<ll::event::entity::ActorEvent>
{
protected:
    ItemActor& mItemActor;

public:
    constexpr explicit ActorPickupItemBeforeEvent(Actor& actor, ItemActor& itemActor)
        : Cancellable(actor)
        , mItemActor(itemActor)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

    ILNDAPI ItemActor& itemActor() const;
};

class ActorPickupItemAfterEvent final : public ll::event::entity::ActorEvent
{
protected:
    ItemActor const& mItemActor;

public:
    constexpr explicit ActorPickupItemAfterEvent(Actor& actor, ItemActor const& itemActor)
        : ActorEvent(actor)
        , mItemActor(itemActor)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

    ILNDAPI ItemActor const& itemActor() const;
};
} // namespace ila::mc::inline world::inline actor
