#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/player/ServerPlayerEvent.h>


// clang-format off
class ItemStack;
// clang-format on

namespace ila::mc::inline world::inline actor::inline player
{
class PlayerAteBeforeEvent final : public ll::event::Cancellable<ll::event::ServerPlayerEvent>
{
protected:
    ItemStack& mItem;

public:
    constexpr explicit PlayerAteBeforeEvent(ServerPlayer& player, ItemStack& item)
        : ll::event::Cancellable<ll::event::ServerPlayerEvent>(player)
        , mItem(item)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

    ILNDAPI ItemStack& item() const;
};
class PlayerAteAfterEvent final : public ll::event::ServerPlayerEvent
{
protected:
    int mSlot;

public:
    constexpr explicit PlayerAteAfterEvent(ServerPlayer& player, int slot)
        : ll::event::ServerPlayerEvent(player)
        , mSlot(slot)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

    ILNDAPI int slot() const;
};
} // namespace ila::mc::inline world::inline actor::inline player