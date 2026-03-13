#include "ila/event/actor/player/PlayerEditSignEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/player/ServerPlayerEvent.h>
#include <ll/api/memory/Hook.h>
#include <magic_enum.hpp>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/nbt/StringTag.h>
#include <mc/nbt/Tag.h>
#include <mc/network/NetEventCallback.h>
#include <mc/network/NetworkBlockPosition.h>
#include <mc/network/NetworkIdentifier.h>
#include <mc/network/Packet/BlockActorDataPacket.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/block/actor/SignBlockActor.h>
#include <mc/world/level/block/actor/SignTextSide.h>
#include <memory>
#include <string>

namespace ila::mc::inline actor::inline player
{

void PlayerEditSignBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]  = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["side"] = magic_enum::enum_name(mTextSide);
    nbt["text"] = mText;
}
void PlayerEditSignBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mPos.x = nbt["pos"][0];
    mPos.y = nbt["pos"][1];
    mPos.z = nbt["pos"][2];
    mText  = nbt["text"];
}

void PlayerEditSignAfterEvent::serialize(CompoundTag& nbt) const
{
    ServerPlayerEvent::serialize(nbt);
    nbt["pos"]  = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["side"] = magic_enum::enum_name(mTextSide);
    nbt["text"] = mText;
}

LL_TYPE_INSTANCE_HOOK(
    PlayerEditSignEventHook,
    HookPriority::Normal,
    ServerNetworkHandler,
    &ServerNetworkHandler::$handle,
    void,
    NetworkIdentifier const&              pSource,
    std::shared_ptr<BlockActorDataPacket> pPacket
)
{
    if (pPacket == nullptr || !pPacket->mData->contains("id", Tag::Type::String)
        || (*pPacket->mData)["id"] != "Sign")
        return origin(pSource, pPacket);

    auto* player = thisFor<NetEventCallback>()->_getServerPlayer(pSource, pPacket->mSenderSubId);
    if (!player) return origin(pSource, pPacket);

    auto* blockActor =
        static_cast<SignBlockActor*>(player->getDimensionBlockSource().getBlockEntity(pPacket->mPos));
    if (blockActor == nullptr) return origin(pSource, pPacket);

    bool frontEdit = false;
    bool backEdit  = false;

    if (blockActor->mTextFront->getMessage() != pPacket->mData.get()["FrontText"]["Text"])
    {
        frontEdit        = true;
        auto beforeEvent = PlayerEditSignBeforeEvent(
            *player,
            pPacket->mPos,
            (*pPacket->mData)["FrontText"]["Text"].get<StringTag>(),
            SignTextSide::Front
        );
        LLEventBus.publish(beforeEvent);
        if (beforeEvent.isCancelled()) return;
    }
    if (blockActor->mTextBack->getMessage() != pPacket->mData.get()["BackText"]["Text"])
    {
        backEdit         = true;
        auto beforeEvent = PlayerEditSignBeforeEvent(
            *player,
            pPacket->mPos,
            (*pPacket->mData)["BackText"]["Text"].get<StringTag>(),
            SignTextSide::Back
        );
        LLEventBus.publish(beforeEvent);
        if (beforeEvent.isCancelled()) return;
    }

    origin(pSource, pPacket);

    if (frontEdit)
    {
        LLEventBus.publish(PlayerEditSignAfterEvent(
            *player,
            pPacket->mPos.get(),
            pPacket->mData.get()["FrontText"]["Text"].get<StringTag>(),
            SignTextSide::Front
        ));
    }
    if (backEdit)
    {
        LLEventBus.publish(PlayerEditSignAfterEvent(
            *player,
            pPacket->mPos.get(),
            pPacket->mData.get()["BackText"]["Text"].get<StringTag>(),
            SignTextSide::Back
        ));
    }
}

Event_Hook_Factory(PlayerEditSign, <PlayerEditSignEventHook>);

} // namespace ila::mc::inline actor::inline player