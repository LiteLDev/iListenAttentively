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
class PlayerStartSleepBeforeEvent final : public ll::event::Cancellable<ll::event::player::PlayerEvent>
{
public:
    BlockPos& mPos;

public:
    constexpr explicit PlayerStartSleepBeforeEvent(Player& player, BlockPos& pos)
        : Cancellable(player)
        , mPos(pos)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class PlayerStartSleepAfterEvent final : public ll::event::player::PlayerEvent
{
public:
    BlockPos const&    mPos;
    BedSleepingResult& mResult;

public:
    constexpr explicit PlayerStartSleepAfterEvent(
        Player&            player,
        BlockPos const&    pos,
        BedSleepingResult& result
    )
        : PlayerEvent(player)
        , mPos(pos)
        , mResult(result)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};
} // namespace ila::mc::inline actor::inline player