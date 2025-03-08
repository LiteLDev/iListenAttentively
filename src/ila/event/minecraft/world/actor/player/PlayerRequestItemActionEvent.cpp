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
    nbt["isDstSerialized"]    = requestAction().mIsDstSerialized;
    nbt["isAmountSerialized"] = requestAction().mIsAmountSerialized;
    nbt["amount"]             = requestAction().mAmount;
    // clang-format off
    nbt["src"] = {
        {"fullContainerName", {
            {"name", magic_enum::enum_name(requestAction().mSrc->mFullContainerName.mName)}
        }},
        {"slot", requestAction().mSrc->mSlot}
    };
    nbt["dst"] = {
        {"fullContainerName", {
            {"name", magic_enum::enum_name(requestAction().mDst->mFullContainerName.mName)}
        }},
        {"slot", requestAction().mDst->mSlot}
    };
    // clang-format on
    if (requestAction().mSrc->mFullContainerName.mDynamicId->has_value())
    {
        nbt["src"]["fullContainerName"]["dynamicId"] =
            requestAction().mSrc->mFullContainerName.mDynamicId->value();
    }
    if (requestAction().mDst->mFullContainerName.mDynamicId->has_value())
    {
        nbt["dst"]["fullContainerName"]["dynamicId"] =
            requestAction().mDst->mFullContainerName.mDynamicId->value();
    }
}
void PlayerRequestItemActionBeforeEvent::deserialize(CompoundTag const& nbt)
{
    PlayerEvent::deserialize(nbt);
    requestAction().mIsDstSerialized    = nbt["isDstSerialized"];
    requestAction().mIsAmountSerialized = nbt["isAmountSerialized"];
    requestAction().mAmount             = nbt["amount"];
    requestAction().mSrc->mFullContainerName.mName =
        magic_enum::enum_cast<ContainerEnumName>(nbt["src"]["fullContainerName"]["name"].get<StringTag>())
            .value_or(requestAction().mSrc->mFullContainerName.mName);
    if (nbt["src"]["fullContainerName"].contains("dynamicId"))
    {
        requestAction().mSrc->mFullContainerName.mDynamicId = nbt["src"]["fullContainerName"]["dynamicId"];
    }
    requestAction().mSrc->mSlot = nbt["src"]["slot"];
    requestAction().mDst->mFullContainerName.mName =
        magic_enum::enum_cast<ContainerEnumName>(nbt["dst"]["fullContainerName"]["name"].get<StringTag>())
            .value_or(requestAction().mDst->mFullContainerName.mName);
    if (nbt["dst"]["fullContainerName"].contains("dynamicId"))
    {
        requestAction().mDst->mFullContainerName.mDynamicId = nbt["dst"]["fullContainerName"]["dynamicId"];
    }
    requestAction().mDst->mSlot = nbt["dst"]["slot"];
}
ItemStackRequestActionTransferBase& PlayerRequestItemActionBeforeEvent::requestAction() const
{
    return mRequestAction;
}

void PlayerRequestItemActionAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["isDstSerialized"]    = requestAction().mIsDstSerialized;
    nbt["isAmountSerialized"] = requestAction().mIsAmountSerialized;
    nbt["amount"]             = requestAction().mAmount;
    nbt["result"]             = magic_enum::enum_name(result());
    // clang-format off
    nbt["src"] = {
        {"fullContainerName", {
            {"name", magic_enum::enum_name(requestAction().mSrc->mFullContainerName.mName)}
        }},
        {"slot", requestAction().mSrc->mSlot}
    };
    nbt["dst"] = {
        {"fullContainerName", {
            {"name", magic_enum::enum_name(requestAction().mDst->mFullContainerName.mName)}
        }},
        {"slot", requestAction().mDst->mSlot}
    };
    // clang-format on
    if (requestAction().mSrc->mFullContainerName.mDynamicId->has_value())
    {
        nbt["src"]["fullContainerName"]["dynamicId"] =
            requestAction().mSrc->mFullContainerName.mDynamicId->value();
    }
    if (requestAction().mDst->mFullContainerName.mDynamicId->has_value())
    {
        nbt["dst"]["fullContainerName"]["dynamicId"] =
            requestAction().mDst->mFullContainerName.mDynamicId->value();
    }
}
void PlayerRequestItemActionAfterEvent::deserialize(CompoundTag const& nbt)
{
    PlayerEvent::deserialize(nbt);
    result() =
        magic_enum::enum_cast<ItemStackNetResult>(nbt["result"].get<StringTag>()).value_or(result());
}
ItemStackRequestActionTransferBase const& PlayerRequestItemActionAfterEvent::requestAction() const
{
    return mRequestAction;
}
ItemStackNetResult& PlayerRequestItemActionAfterEvent::result() const { return mResult; }

LL_TYPE_INSTANCE_HOOK(
    PlayerRequestItemActionEventHook,
    HookPriority::Normal,
    ItemStackRequestActionHandler,
    &ItemStackRequestActionHandler::handleRequestAction,
    ItemStackNetResult,
    ItemStackRequestAction const& pRequestAction
)
{
    auto beforeEvent = PlayerRequestItemActionBeforeEvent(
        mPlayer,
        static_cast<ItemStackRequestActionTransferBase&>(const_cast<ItemStackRequestAction&>(pRequestAction))
    );
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return ItemStackNetResult::Error; }
    auto result = origin(pRequestAction);
    LLEventBus.publish(PlayerRequestItemActionAfterEvent(
        mPlayer,
        static_cast<ItemStackRequestActionTransferBase const&>(pRequestAction),
        result
    ));
    return result;
}

Event_Hook_Factory(PlayerRequestItemAction, <PlayerRequestItemActionEventHook>);

} // namespace ila::mc::inline world::inline actor::inline player