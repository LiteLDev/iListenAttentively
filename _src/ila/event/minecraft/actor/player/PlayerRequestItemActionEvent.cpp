#include "ila/event/actor/player/PlayerRequestItemActionEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/base/StdInt.h>
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/player/PlayerEvent.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/memory/Memory.h>
#include <magic_enum.hpp>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/StringTag.h>
#include <mc/world/containers/ContainerEnumName.h>
#include <mc/world/containers/FullContainerName.h>
#include <mc/world/inventory/network/ItemStackNetResult.h>
#include <mc/world/inventory/network/ItemStackRequestAction.h>
#include <mc/world/inventory/network/ItemStackRequestActionHandler.h>
#include <mc/world/inventory/network/ItemStackRequestActionTransferBase.h>
#include <mc/world/inventory/network/ItemStackRequestActionType.h>
#include <mc/world/inventory/network/ItemStackRequestSlotInfo.h>

namespace ila::mc::inline actor::inline player
{

void PlayerRequestItemActionBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["actionType"]         = magic_enum::enum_name(mActionType);
    nbt["isDstSerialized"]    = mIsDstSerialized;
    nbt["isAmountSerialized"] = mIsAmountSerialized;
    nbt["amount"]             = mAmount;
    // clang-format off
    nbt["src"] = {
        {"fullContainerName", {
            {"name", magic_enum::enum_name(mSrc.mFullContainerName.mName)}
        }},
        {"slot", mSrc.mSlot}
    };
    nbt["dst"] = {
        {"fullContainerName", {
            {"name", magic_enum::enum_name(mDst.mFullContainerName.mName)}
        }},
        {"slot", mDst.mSlot}
    };
    // clang-format on
    if (mSrc.mFullContainerName.mDynamicId->has_value())
    {
        nbt["src"]["fullContainerName"]["dynamicId"] = mSrc.mFullContainerName.mDynamicId->value();
    }
    if (mDst.mFullContainerName.mDynamicId->has_value())
    {
        nbt["dst"]["fullContainerName"]["dynamicId"] = mDst.mFullContainerName.mDynamicId->value();
    }
}
void PlayerRequestItemActionBeforeEvent::deserialize(CompoundTag const& nbt)
{
    PlayerEvent::deserialize(nbt);
    mActionType = magic_enum::enum_cast<ItemStackRequestActionType>(nbt["actionType"].get<StringTag>())
                       .value_or(mActionType);
    mIsDstSerialized    = nbt["isDstSerialized"];
    mIsAmountSerialized = nbt["isAmountSerialized"];
    mAmount             = nbt["amount"];
    mSrc.mFullContainerName.mName =
        magic_enum::enum_cast<ContainerEnumName>(nbt["src"]["fullContainerName"]["name"].get<StringTag>())
            .value_or(mSrc.mFullContainerName.mName);
    if (nbt["src"]["fullContainerName"].contains("dynamicId"))
    {
        mSrc.mFullContainerName.mDynamicId = nbt["src"]["fullContainerName"]["dynamicId"];
    }
    else { mSrc.mFullContainerName.mDynamicId->reset(); }
    mSrc.mSlot = nbt["src"]["slot"];
    mDst.mFullContainerName.mName =
        magic_enum::enum_cast<ContainerEnumName>(nbt["dst"]["fullContainerName"]["name"].get<StringTag>())
            .value_or(mDst.mFullContainerName.mName);
    if (nbt["dst"]["fullContainerName"].contains("dynamicId"))
    {
        mDst.mFullContainerName.mDynamicId = nbt["dst"]["fullContainerName"]["dynamicId"];
    }
    else { mDst.mFullContainerName.mDynamicId->reset(); }
    mDst.mSlot = nbt["dst"]["slot"];
}

void PlayerRequestItemActionAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["actionType"]         = magic_enum::enum_name(mActionType);
    nbt["isDstSerialized"]    = mIsDstSerialized;
    nbt["isAmountSerialized"] = mIsAmountSerialized;
    nbt["amount"]             = mAmount;
    nbt["result"]             = magic_enum::enum_name(mResult);
    // clang-format off
    nbt["src"] = {
        {"fullContainerName", {
            {"name", magic_enum::enum_name(mSrc.mFullContainerName.mName)}
        }},
        {"slot", mSrc.mSlot}
    };
    nbt["dst"] = {
        {"fullContainerName", {
            {"name", magic_enum::enum_name(mDst.mFullContainerName.mName)}
        }},
        {"slot", mDst.mSlot}
    };
    // clang-format on
    if (mSrc.mFullContainerName.mDynamicId->has_value())
    {
        nbt["src"]["fullContainerName"]["dynamicId"] = mSrc.mFullContainerName.mDynamicId->value();
    }
    if (mDst.mFullContainerName.mDynamicId->has_value())
    {
        nbt["dst"]["fullContainerName"]["dynamicId"] = mDst.mFullContainerName.mDynamicId->value();
    }
}
void PlayerRequestItemActionAfterEvent::deserialize(CompoundTag const& nbt)
{
    PlayerEvent::deserialize(nbt);
    mResult = magic_enum::enum_cast<ItemStackNetResult>(nbt["result"].get<StringTag>()).value_or(mResult);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerRequestItemActionEventHook,
    HookPriority::Normal,
    ItemStackRequestActionHandler,
    &ItemStackRequestActionHandler::handleRequestAction,
    ItemStackNetResult,
    ItemStackRequestAction const& pRequestAction
)
{
    auto& action =
        static_cast<ItemStackRequestActionTransferBase&>(const_cast<ItemStackRequestAction&>(pRequestAction));
    auto beforeEvent = PlayerRequestItemActionBeforeEvent(
        mPlayer,
        action.mActionType,
        action.mIsAmountSerialized,
        action.mIsAmountSerialized,
        action.mAmount,
        dAccess<ItemStackRequestSlotInfo>(&action.mSrc, 4),
        dAccess<ItemStackRequestSlotInfo>(&action.mDst, 4)
    );
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return ItemStackNetResult::Error; }
    auto result = origin(pRequestAction);
    LLEventBus.publish(PlayerRequestItemActionAfterEvent(
        mPlayer,
        action.mActionType,
        action.mIsAmountSerialized,
        action.mIsAmountSerialized,
        action.mAmount,
        dAccess<ItemStackRequestSlotInfo>(&action.mSrc, 4),
        dAccess<ItemStackRequestSlotInfo>(&action.mDst, 4),
        result
    ));
    return result;
}

Event_Hook_Factory(PlayerRequestItemAction, <PlayerRequestItemActionEventHook>);

} // namespace ila::mc::inline actor::inline player