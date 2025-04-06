#include "ila/event/minecraft/world/actor/player/PlayerRequestItemActionEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/containers/FullContainerName.h>
#include <mc/world/inventory/network/ItemStackRequestActionHandler.h>
#include <mc/world/inventory/network/ItemStackRequestSlotInfo.h>

namespace ila::mc::inline world::inline actor::inline player
{

void PlayerRequestItemActionBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["actionType"]         = magic_enum::enum_name(actionType());
    nbt["isDstSerialized"]    = isDstSerialized();
    nbt["isAmountSerialized"] = isAmountSerialized();
    nbt["amount"]             = amount();
    // clang-format off
    nbt["src"] = {
        {"fullContainerName", {
            {"name", magic_enum::enum_name(src().mFullContainerName.mName)}
        }},
        {"slot", src().mSlot}
    };
    nbt["dst"] = {
        {"fullContainerName", {
            {"name", magic_enum::enum_name(dst().mFullContainerName.mName)}
        }},
        {"slot", dst().mSlot}
    };
    // clang-format on
    if (src().mFullContainerName.mDynamicId->has_value())
    {
        nbt["src"]["fullContainerName"]["dynamicId"] = src().mFullContainerName.mDynamicId->value();
    }
    if (dst().mFullContainerName.mDynamicId->has_value())
    {
        nbt["dst"]["fullContainerName"]["dynamicId"] = dst().mFullContainerName.mDynamicId->value();
    }
}
void PlayerRequestItemActionBeforeEvent::deserialize(CompoundTag const& nbt)
{
    PlayerEvent::deserialize(nbt);
    actionType() = magic_enum::enum_cast<ItemStackRequestActionType>(nbt["actionType"].get<StringTag>())
                       .value_or(actionType());
    isDstSerialized()    = nbt["isDstSerialized"];
    isAmountSerialized() = nbt["isAmountSerialized"];
    amount()             = nbt["amount"];
    src().mFullContainerName.mName =
        magic_enum::enum_cast<ContainerEnumName>(nbt["src"]["fullContainerName"]["name"].get<StringTag>())
            .value_or(src().mFullContainerName.mName);
    if (nbt["src"]["fullContainerName"].contains("dynamicId"))
    {
        src().mFullContainerName.mDynamicId = nbt["src"]["fullContainerName"]["dynamicId"];
    }
    else { src().mFullContainerName.mDynamicId->reset(); }
    src().mSlot = nbt["src"]["slot"];
    dst().mFullContainerName.mName =
        magic_enum::enum_cast<ContainerEnumName>(nbt["dst"]["fullContainerName"]["name"].get<StringTag>())
            .value_or(dst().mFullContainerName.mName);
    if (nbt["dst"]["fullContainerName"].contains("dynamicId"))
    {
        dst().mFullContainerName.mDynamicId = nbt["dst"]["fullContainerName"]["dynamicId"];
    }
    else { dst().mFullContainerName.mDynamicId->reset(); }
    dst().mSlot = nbt["dst"]["slot"];
}
ItemStackRequestActionType& PlayerRequestItemActionBeforeEvent::actionType() const { return mActionType; }
bool&  PlayerRequestItemActionBeforeEvent::isDstSerialized() const { return mIsDstSerialized; }
bool&  PlayerRequestItemActionBeforeEvent::isAmountSerialized() const { return mIsAmountSerialized; }
uchar& PlayerRequestItemActionBeforeEvent::amount() const { return mAmount; }
ItemStackRequestSlotInfo& PlayerRequestItemActionBeforeEvent::src() const { return mSrc; }
ItemStackRequestSlotInfo& PlayerRequestItemActionBeforeEvent::dst() const { return mDst; }

void PlayerRequestItemActionAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["actionType"]         = magic_enum::enum_name(actionType());
    nbt["isDstSerialized"]    = isDstSerialized();
    nbt["isAmountSerialized"] = isAmountSerialized();
    nbt["amount"]             = amount();
    nbt["result"]             = magic_enum::enum_name(result());
    // clang-format off
    nbt["src"] = {
        {"fullContainerName", {
            {"name", magic_enum::enum_name(src().mFullContainerName.mName)}
        }},
        {"slot", src().mSlot}
    };
    nbt["dst"] = {
        {"fullContainerName", {
            {"name", magic_enum::enum_name(dst().mFullContainerName.mName)}
        }},
        {"slot", dst().mSlot}
    };
    // clang-format on
    if (src().mFullContainerName.mDynamicId->has_value())
    {
        nbt["src"]["fullContainerName"]["dynamicId"] = src().mFullContainerName.mDynamicId->value();
    }
    if (dst().mFullContainerName.mDynamicId->has_value())
    {
        nbt["dst"]["fullContainerName"]["dynamicId"] = dst().mFullContainerName.mDynamicId->value();
    }
}
void PlayerRequestItemActionAfterEvent::deserialize(CompoundTag const& nbt)
{
    PlayerEvent::deserialize(nbt);
    result() = magic_enum::enum_cast<ItemStackNetResult>(nbt["result"].get<StringTag>()).value_or(result());
}
ItemStackRequestActionType const& PlayerRequestItemActionAfterEvent::actionType() const
{
    return mActionType;
}
bool const&  PlayerRequestItemActionAfterEvent::isDstSerialized() const { return mIsDstSerialized; }
bool const&  PlayerRequestItemActionAfterEvent::isAmountSerialized() const { return mIsAmountSerialized; }
uchar const& PlayerRequestItemActionAfterEvent::amount() const { return mAmount; }
ItemStackRequestSlotInfo const& PlayerRequestItemActionAfterEvent::src() const { return mSrc; }
ItemStackRequestSlotInfo const& PlayerRequestItemActionAfterEvent::dst() const { return mDst; }
ItemStackNetResult&             PlayerRequestItemActionAfterEvent::result() const { return mResult; }

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

} // namespace ila::mc::inline world::inline actor::inline player