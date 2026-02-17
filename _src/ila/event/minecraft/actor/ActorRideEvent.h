#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/entity/ActorEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/Actor.h>

namespace ila::mc::inline actor
{
class ActorRideBeforeEvent final : public ll::event::Cancellable<ll::event::entity::ActorEvent>
{
public:
    Actor& mTarget;

public:
    constexpr explicit ActorRideBeforeEvent(Actor& actor, Actor& target)
        : Cancellable(actor)
        , mTarget(target)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};

class ActorRideAfterEvent final : public ll::event::entity::ActorEvent
{
public:
    Actor const& mTarget;

public:
    constexpr explicit ActorRideAfterEvent(Actor& actor, Actor const& target)
        : ActorEvent(actor)
        , mTarget(target)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline actor