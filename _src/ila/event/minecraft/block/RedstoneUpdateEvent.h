#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/world/WorldEvent.h>

// clang-format off
class BlockPos;
// clang-format on

namespace ila::mc::inline block
{
class RedstoneUpdateBeforeEvent final : public ll::event::Cancellable<ll::event::WorldEvent>
{
public:
    BlockPos& mPos;
    int&      mStrength;
    bool&     mIsFirstTime;

public:
    constexpr explicit RedstoneUpdateBeforeEvent(
        BlockSource& blockSource,
        BlockPos&    pos,
        int&         strength,
        bool&        isFirstTime
    )
        : Cancellable(blockSource)
        , mPos(pos)
        , mStrength(strength)
        , mIsFirstTime(isFirstTime)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class RedstoneUpdateAfterEvent final : public ll::event::WorldEvent
{
public:
    BlockPos const& mPos;
    int const&      mStrength;
    bool const&     mIsFirstTime;

public:
    constexpr explicit RedstoneUpdateAfterEvent(
        BlockSource&    blockSource,
        BlockPos const& pos,
        int const&      strength,
        bool const&     isFirstTime
    )
        : WorldEvent(blockSource)
        , mPos(pos)
        , mStrength(strength)
        , mIsFirstTime(isFirstTime)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};
} // namespace ila::mc::inline block
