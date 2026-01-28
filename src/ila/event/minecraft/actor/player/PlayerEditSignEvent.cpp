#include "ila/event/minecraft/actor/player/PlayerEditSignEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/network/NetworkBlockPosition.h>
#include <mc/network/Packet/BlockActorDataPacket.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/block/actor/SignBlockActor.h>

namespace ila::mc::inline world::inline actor::inline player
{

void PlayerEditSignBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]  = ListTag { pos().x, pos().y, pos().z };
    nbt["side"] = magic_enum::enum_name(textSide());
    nbt["text"] = text();
}
void PlayerEditSignBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x = nbt["pos"][0];
    pos().y = nbt["pos"][1];
    pos().z = nbt["pos"][2];
    text()  = nbt["text"];
}
BlockPos&           PlayerEditSignBeforeEvent::pos() const { return mPos; }
SignTextSide const& PlayerEditSignBeforeEvent::textSide() const { return mTextSide; }
std::string&        PlayerEditSignBeforeEvent::text() const { return mText; }

void PlayerEditSignAfterEvent::serialize(CompoundTag& nbt) const
{
    ServerPlayerEvent::serialize(nbt);
    nbt["pos"]  = ListTag { pos().x, pos().y, pos().z };
    nbt["side"] = magic_enum::enum_name(textSide());
    nbt["text"] = text();
}
BlockPos const&     PlayerEditSignAfterEvent::pos() const { return mPos; }
SignTextSide const& PlayerEditSignAfterEvent::textSide() const { return mTextSide; }
std::string const&  PlayerEditSignAfterEvent::text() const { return mText; }

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

} // namespace ila::mc::inline world::inline actor::inline player