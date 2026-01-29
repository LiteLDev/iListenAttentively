#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/player/ServerPlayerEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/server/ServerPlayer.h>
#include <mc/world/level/block/actor/SignTextSide.h>
#include <string>


// clang-format off
class BlockPos;
// clang-format on

namespace ila::mc::inline actor::inline player
{
class PlayerEditSignBeforeEvent final : public ll::event::Cancellable<ll::event::player::ServerPlayerEvent>
{
public:
    BlockPos&           mPos;
    std::string&        mText;
    SignTextSide const& mTextSide;

public:
    constexpr explicit PlayerEditSignBeforeEvent(
        ServerPlayer&       player,
        BlockPos&           pos,
        std::string&        text,
        SignTextSide const& textSide
    )
        : Cancellable(player)
        , mPos(pos)
        , mText(text)
        , mTextSide(textSide)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class PlayerEditSignAfterEvent final : public ll::event::player::ServerPlayerEvent
{
public:
    BlockPos const&     mPos;
    std::string const&  mText;
    SignTextSide const& mTextSide;

public:
    constexpr explicit PlayerEditSignAfterEvent(
        ServerPlayer&       player,
        BlockPos const&     pos,
        std::string const&  text,
        SignTextSide const& textSide
    )
        : ServerPlayerEvent(player)
        , mPos(pos)
        , mText(text)
        , mTextSide(textSide)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline actor::inline player