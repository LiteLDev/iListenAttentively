#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/entity/ActorEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/Actor.h>

// clang-format off
class Vec3;
// clang-format on

namespace ila::mc::inline actor
{
class ActorDestroyBlockEvent final : public ll::event::Cancellable<ll::event::entity::ActorEvent>
{
public:
    Vec3 const& mPos;

public:
    constexpr explicit ActorDestroyBlockEvent(Actor& actor, Vec3 const& pos)
        : Cancellable(actor)
        , mPos(pos)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline actor