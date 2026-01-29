#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/entity/ActorEvent.h>
#include <ll/api/event/player/PlayerEvent.h>
#include <mc/deps/shared_types/legacy/EquipmentSlot.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/player/Player.h>

namespace ila::mc::inline actor
{
class ArmorStandSwapItemBeforeEvent final : public ll::event::Cancellable<ll::event::entity::ActorEvent>
{
protected:
    Player&                             mPlayer;
    SharedTypes::Legacy::EquipmentSlot& mSlot;

public:
    constexpr explicit ArmorStandSwapItemBeforeEvent(
        Actor&                              actor,
        Player&                             player,
        SharedTypes::Legacy::EquipmentSlot& slot
    )
        : Cancellable(actor)
        , mPlayer(player)
        , mSlot(slot)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

    ILNDAPI Player& player() const;
    ILNDAPI SharedTypes::Legacy::EquipmentSlot& slot() const;
};

class ArmorStandSwapItemAfterEvent final : public ll::event::entity::ActorEvent
{
protected:
    Player const&                             mPlayer;
    SharedTypes::Legacy::EquipmentSlot const& mSlot;

public:
    constexpr explicit ArmorStandSwapItemAfterEvent(
        Actor&                                    actor,
        Player const&                             player,
        SharedTypes::Legacy::EquipmentSlot const& slot
    )
        : ActorEvent(actor)
        , mPlayer(player)
        , mSlot(slot)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

    ILNDAPI Player const& player() const;
    ILNDAPI SharedTypes::Legacy::EquipmentSlot const& slot() const;
};
} // namespace ila::mc::inline actor