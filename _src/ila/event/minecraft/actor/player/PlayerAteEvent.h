#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/player/PlayerEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/player/Player.h>

// clang-format off
class ItemStack;
// clang-format on

namespace ila::mc::inline actor::inline player
{
class PlayerAteBeforeEvent final : public ll::event::Cancellable<ll::event::PlayerEvent>
{
public:
    ItemStack& mItem;

public:
    constexpr explicit PlayerAteBeforeEvent(Player& player, ItemStack& item)
        : Cancellable(player)
        , mItem(item)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};

class PlayerAteAfterEvent final : public ll::event::PlayerEvent
{
public:
    int mSlot;

public:
    constexpr explicit PlayerAteAfterEvent(Player& player, int slot)
        : PlayerEvent(player)
        , mSlot(slot)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline actor::inline player