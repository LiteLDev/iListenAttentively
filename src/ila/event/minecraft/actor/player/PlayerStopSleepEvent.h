#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/player/PlayerEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/player/BedSleepingResult.h>
#include <mc/world/actor/player/Player.h>

// clang-format off
class BlockPos;
// clang-format on


namespace ila::mc::inline actor::inline player
{
/**
  When a player joins the server, if existing player data is found in the save file,
  the server will load the player data and trigger this event.
  However, member variables such as the player"s dimension are not yet initialized at this stage.
  You need to implement custom checks accordingly.
**/
class PlayerStopSleepBeforeEvent final : public ll::event::player::PlayerEvent
{
protected:
    bool& mForcefulWakeUp;
    bool& mUpdateLevelList;

public:
    constexpr explicit PlayerStopSleepBeforeEvent(Player& player, bool& forcefulWakeUp, bool& updateLevelList)
        : PlayerEvent(player)
        , mForcefulWakeUp(forcefulWakeUp)
        , mUpdateLevelList(updateLevelList)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

    ILNDAPI bool& forcefulWakeUp() const;
    ILNDAPI bool& updateLevelList() const;
};

/**
  When a player joins the server, if existing player data is found in the save file,
  the server will load the player data and trigger this event.
  However, member variables such as the player"s dimension are not yet initialized at this stage.
  You need to implement custom checks accordingly.
**/
class PlayerStopSleepAfterEvent final : public ll::event::player::PlayerEvent
{
protected:
    bool const& mForcefulWakeUp;
    bool const& mUpdateLevelList;

public:
    constexpr explicit PlayerStopSleepAfterEvent(
        Player&     player,
        bool const& forcefulWakeUp,
        bool const& updateLevelList
    )
        : PlayerEvent(player)
        , mForcefulWakeUp(forcefulWakeUp)
        , mUpdateLevelList(updateLevelList)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

    ILNDAPI bool const& forcefulWakeUp() const;
    ILNDAPI bool const& updateLevelList() const;
};
} // namespace ila::mc::inline actor::inline player