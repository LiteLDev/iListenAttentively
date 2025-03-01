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
    nbt["isDstSerialized"]    = getRequestAction().mIsDstSerialized;
    nbt["isAmountSerialized"] = getRequestAction().mIsAmountSerialized;
    nbt["amount"]             = getRequestAction().mAmount;
    // clang-format off
    nbt["src"] = {
        {"fullContainerName", {
            {"name", magic_enum::enum_name(getRequestAction().mSrc->mFullContainerName.mName)}
        }},
        {"slot", getRequestAction().mSrc->mSlot}
    };
    nbt["dst"] = {
        {"fullContainerName", {
            {"name", magic_enum::enum_name(getRequestAction().mDst->mFullContainerName.mName)}
        }},
        {"slot", getRequestAction().mDst->mSlot}
    };
    // clang-format on
    if (getRequestAction().mSrc->mFullContainerName.mDynamicId->has_value())
    {
        nbt["src"]["fullContainerName"]["dynamicId"] =
            getRequestAction().mSrc->mFullContainerName.mDynamicId->value();
    }
    if (getRequestAction().mDst->mFullContainerName.mDynamicId->has_value())
    {
        nbt["dst"]["fullContainerName"]["dynamicId"] =
            getRequestAction().mDst->mFullContainerName.mDynamicId->value();
    }
}
void PlayerRequestItemActionBeforeEvent::deserialize(CompoundTag const& nbt)
{
    PlayerEvent::deserialize(nbt);
    getRequestAction().mIsDstSerialized    = nbt["isDstSerialized"];
    getRequestAction().mIsAmountSerialized = nbt["isAmountSerialized"];
    getRequestAction().mAmount             = nbt["amount"];
    getRequestAction().mSrc->mFullContainerName.mName =
        magic_enum::enum_cast<ContainerEnumName>(nbt["src"]["fullContainerName"]["name"].get<StringTag>())
            .value_or(getRequestAction().mSrc->mFullContainerName.mName);
    if (nbt["src"]["fullContainerName"].contains("dynamicId"))
    {
        getRequestAction().mSrc->mFullContainerName.mDynamicId = nbt["src"]["fullContainerName"]["dynamicId"];
    }
    getRequestAction().mSrc->mSlot = nbt["src"]["slot"];
    getRequestAction().mDst->mFullContainerName.mName =
        magic_enum::enum_cast<ContainerEnumName>(nbt["dst"]["fullContainerName"]["name"].get<StringTag>())
            .value_or(getRequestAction().mDst->mFullContainerName.mName);
    if (nbt["dst"]["fullContainerName"].contains("dynamicId"))
    {
        getRequestAction().mDst->mFullContainerName.mDynamicId = nbt["dst"]["fullContainerName"]["dynamicId"];
    }
    getRequestAction().mDst->mSlot = nbt["dst"]["slot"];
}
ItemStackRequestActionTransferBase& PlayerRequestItemActionBeforeEvent::getRequestAction() const
{
    return mRequestAction;
}

void PlayerRequestItemActionAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["isDstSerialized"]    = getRequestAction().mIsDstSerialized;
    nbt["isAmountSerialized"] = getRequestAction().mIsAmountSerialized;
    nbt["amount"]             = getRequestAction().mAmount;
    nbt["result"]             = magic_enum::enum_name(getResult());
    // clang-format off
    nbt["src"] = {
        {"fullContainerName", {
            {"name", magic_enum::enum_name(getRequestAction().mSrc->mFullContainerName.mName)}
        }},
        {"slot", getRequestAction().mSrc->mSlot}
    };
    nbt["dst"] = {
        {"fullContainerName", {
            {"name", magic_enum::enum_name(getRequestAction().mDst->mFullContainerName.mName)}
        }},
        {"slot", getRequestAction().mDst->mSlot}
    };
    // clang-format on
    if (getRequestAction().mSrc->mFullContainerName.mDynamicId->has_value())
    {
        nbt["src"]["fullContainerName"]["dynamicId"] =
            getRequestAction().mSrc->mFullContainerName.mDynamicId->value();
    }
    if (getRequestAction().mDst->mFullContainerName.mDynamicId->has_value())
    {
        nbt["dst"]["fullContainerName"]["dynamicId"] =
            getRequestAction().mDst->mFullContainerName.mDynamicId->value();
    }
}
void PlayerRequestItemActionAfterEvent::deserialize(CompoundTag const& nbt)
{
    PlayerEvent::deserialize(nbt);
    getResult() =
        magic_enum::enum_cast<ItemStackNetResult>(nbt["result"].get<StringTag>()).value_or(getResult());
}
ItemStackRequestActionTransferBase const& PlayerRequestItemActionAfterEvent::getRequestAction() const
{
    return mRequestAction;
}
ItemStackNetResult& PlayerRequestItemActionAfterEvent::getResult() const { return mResult; }

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