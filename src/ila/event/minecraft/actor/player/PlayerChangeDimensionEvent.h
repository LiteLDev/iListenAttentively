#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/player/PlayerEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/player/Player.h>

// clang-format off
class ChangeDimensionRequest;
class Dimension;
// clang-format on

namespace ila::mc::inline actor::inline player
{
class PlayerChangeDimensionBeforeEvent final : public ll::event::player::PlayerEvent
{
public:
    ChangeDimensionRequest const& mChangeDimensionRequest;
    Dimension const&              mDimension;

public:
    constexpr explicit PlayerChangeDimensionBeforeEvent(
        Player&                       player,
        ChangeDimensionRequest const& changeDimensionRequest,
        Dimension const&              dimension
    )
        : PlayerEvent(player)
        , mChangeDimensionRequest(changeDimensionRequest)
        , mDimension(dimension)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};

class PlayerChangeDimensionAfterEvent final : public ll::event::player::PlayerEvent
{
public:
    ChangeDimensionRequest const& mChangeDimensionRequest;
    Dimension const&              mDimension;

public:
    constexpr explicit PlayerChangeDimensionAfterEvent(
        Player&                       player,
        ChangeDimensionRequest const& changeDimensionRequest,
        Dimension const&              dimension
    )
        : PlayerEvent(player)
        , mChangeDimensionRequest(changeDimensionRequest)
        , mDimension(dimension)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline actor::inline player