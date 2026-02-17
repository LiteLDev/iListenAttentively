#pragma once
#include "ila/base/Macro.h"
#include <ll/api/base/StdInt.h>
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/player/PlayerEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/inventory/network/ItemStackNetResult.h>
#include <mc/world/inventory/network/ItemStackRequestActionType.h>

// clang-format off
struct ItemStackRequestSlotInfo;
// clang-format on

namespace ila::mc::inline actor::inline player
{
class PlayerRequestItemActionBeforeEvent final : public ll::event::Cancellable<ll::event::player::PlayerEvent>
{
public:
    ItemStackRequestActionType& mActionType;
    bool&                       mIsDstSerialized;
    bool&                       mIsAmountSerialized;
    uchar&                      mAmount;
    ItemStackRequestSlotInfo&   mSrc;
    ItemStackRequestSlotInfo&   mDst;

public:
    constexpr explicit PlayerRequestItemActionBeforeEvent(
        Player&                     player,
        ItemStackRequestActionType& actionType,
        bool&                       isDstSerialized,
        bool&                       isAmountSerialized,
        uchar&                      amount,
        ItemStackRequestSlotInfo&   src,
        ItemStackRequestSlotInfo&   dst
    )
        : Cancellable(player)
        , mActionType(actionType)
        , mIsDstSerialized(isDstSerialized)
        , mIsAmountSerialized(isAmountSerialized)
        , mAmount(amount)
        , mSrc(src)
        , mDst(dst)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class PlayerRequestItemActionAfterEvent final : public ll::event::player::PlayerEvent
{
public:
    ItemStackRequestActionType const& mActionType;
    bool const&                       mIsDstSerialized;
    bool const&                       mIsAmountSerialized;
    uchar const&                      mAmount;
    ItemStackRequestSlotInfo const&   mSrc;
    ItemStackRequestSlotInfo const&   mDst;
    ItemStackNetResult&               mResult;

public:
    constexpr explicit PlayerRequestItemActionAfterEvent(
        Player&                           player,
        ItemStackRequestActionType const& actionType,
        bool const&                       isDstSerialized,
        bool const&                       isAmountSerialized,
        uchar const&                      amount,
        ItemStackRequestSlotInfo const&   src,
        ItemStackRequestSlotInfo const&   dst,
        ItemStackNetResult&               result
    )
        : PlayerEvent(player)
        , mActionType(actionType)
        , mIsDstSerialized(isDstSerialized)
        , mIsAmountSerialized(isAmountSerialized)
        , mAmount(amount)
        , mSrc(src)
        , mDst(dst)
        , mResult(result)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};
} // namespace ila::mc::inline actor::inline player