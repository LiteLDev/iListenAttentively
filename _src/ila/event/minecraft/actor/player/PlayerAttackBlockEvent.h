#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/player/ServerPlayerEvent.h>
#include <mc/common/FacingID.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/server/ServerPlayer.h>

// clang-format off
class BlockPos;
// clang-format on

namespace ila::mc::inline actor::inline player
{
class PlayerAttackBlockBeforeEvent final : public ll::event::Cancellable<ll::event::player::ServerPlayerEvent>
{
public:
    BlockPos& mPos;
    FacingID& mFace;

public:
    constexpr explicit PlayerAttackBlockBeforeEvent(ServerPlayer& player, BlockPos& pos, FacingID& face)
        : Cancellable(player)
        , mPos(pos)
        , mFace(face)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class PlayerAttackBlockAfterEvent final : public ll::event::player::ServerPlayerEvent
{
public:
    BlockPos const& mPos;
    FacingID const& mFace;

public:
    constexpr explicit PlayerAttackBlockAfterEvent(
        ServerPlayer&   player,
        BlockPos const& pos,
        FacingID const& face
    )
        : ServerPlayerEvent(player)
        , mPos(pos)
        , mFace(face)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline actor::inline player