#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/entity/ActorEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/Actor.h>

// clang-format off
class MobEffectInstance;
// clang-format on

namespace ila::mc::inline actor
{
class ActorGetEffectBeforeEvent final : public ll::event::Cancellable<ll::event::entity::ActorEvent>
{
public:
    MobEffectInstance& mEffect;

public:
    constexpr explicit ActorGetEffectBeforeEvent(Actor& actor, MobEffectInstance& effect)
        : Cancellable(actor)
        , mEffect(effect)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class ActorGetEffectAfterEvent final : public ll::event::entity::ActorEvent
{
public:
    MobEffectInstance const& mEffect;

public:
    constexpr explicit ActorGetEffectAfterEvent(Actor& actor, MobEffectInstance const& effect)
        : ActorEvent(actor)
        , mEffect(effect)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline actor