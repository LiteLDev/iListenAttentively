#pragma include_alias("mc/world/level/block/states/BlockStateVariant.h", "ila/patch/BlockStateVariant.hpp")
#include "ila/event/minecraft/actor/player/PlayerOperatedItemFrameEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/item/ItemInstance.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/block/BlockType.h>
#include <mc/world/level/block/ItemFrameBlock.h>
#include <mc/world/level/block/actor/ItemFrameBlockActor.h>
#include <mc/world/level/block/block_events/BlockPlaceEvent.h>
#include <mc/world/level/block/block_events/BlockPlayerInteractEvent.h>

namespace ila::mc::inline world::inline actor::inline player
{

void PlayerOperatedItemFrameBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]  = ListTag { blockPos().x, blockPos().y, blockPos().z };
    nbt["type"] = magic_enum::enum_name(type());
}
BlockPos const& PlayerOperatedItemFrameBeforeEvent::blockPos() const { return mBlockPos; }
PlayerOperatedItemFrameEvent::Type const& PlayerOperatedItemFrameBeforeEvent::type() const { return mType; }

void PlayerOperatedItemFrameAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["pos"]  = ListTag { blockPos().x, blockPos().y, blockPos().z };
    nbt["type"] = magic_enum::enum_name(type());
}
BlockPos const& PlayerOperatedItemFrameAfterEvent::blockPos() const { return mBlockPos; }
PlayerOperatedItemFrameEvent::Type const& PlayerOperatedItemFrameAfterEvent::type() const { return mType; }

using Type = PlayerOperatedItemFrameEvent::Type;

LL_TYPE_INSTANCE_HOOK(
    PlayerOperatedItemFrameEventHook1,
    HookPriority::Normal,
    ItemFrameBlock,
    &ItemFrameBlock::use,
    void,
    BlockEvents::BlockPlayerInteractEvent& pEventData
)
{
    auto* blockActor = static_cast<ItemFrameBlockActor*>(
        pEventData.mPlayer.getDimensionBlockSource().getBlockEntity(pEventData.mPos)
    );
    if (!blockActor) { return origin(pEventData); }
    auto type = blockActor->mItem->isNull() ? Type::Place : Type::Rotate;
    if (type == Type::Place && pEventData.mPlayer.getSelectedItem().isNull()) { return origin(pEventData); }
    auto beforeEvent = PlayerOperatedItemFrameBeforeEvent(pEventData.mPlayer, pEventData.mPos, type);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pEventData);
    LLEventBus.publish(PlayerOperatedItemFrameAfterEvent(pEventData.mPlayer, pEventData.mPos, type));
}

LL_TYPE_INSTANCE_HOOK(
    PlayerOperatedItemFrameEventHook2,
    HookPriority::Normal,
    ItemFrameBlock,
    &ItemFrameBlock::$attack,
    bool,
    Player*         pPlayer,
    BlockPos const& pPos
)
{
    if (pPlayer == nullptr) { return origin(pPlayer, pPos); }
    if (auto* blockEntity = pPlayer->getDimensionBlockSource().getBlockEntity(pPos);
        !blockEntity || static_cast<ItemFrameBlockActor*>(blockEntity)->mItem->isNull())
    {
        return origin(pPlayer, pPos);
    }
    auto beforeEvent = PlayerOperatedItemFrameBeforeEvent(*pPlayer, const_cast<BlockPos&>(pPos), Type::Take);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return false; }
    auto result = origin(pPlayer, pPos);
    if (result) { LLEventBus.publish(PlayerOperatedItemFrameAfterEvent(*pPlayer, pPos, Type::Take)); }
    return result;
}

LL_TYPE_INSTANCE_HOOK(
    PlayerOperatedItemFrameEventHook3,
    HookPriority::Normal,
    ItemFrameBlockActor,
    &ItemFrameBlockActor::dropFramedItem,
    void,
    BlockSource& pRegion,
    bool         pIsSurvival,
    Actor*       pActor
)
{
    if (pActor == nullptr || !pActor->isPlayer() || !pActor->isCreative())
    {
        return origin(pRegion, pIsSurvival, pActor);
    }
    auto beforeEvent =
        PlayerOperatedItemFrameBeforeEvent(static_cast<Player&>(*pActor), mPosition, Type::Take);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pRegion, pIsSurvival, pActor);
    LLEventBus.publish(PlayerOperatedItemFrameAfterEvent(static_cast<Player&>(*pActor), mPosition, Type::Take)
    );
}

Event_Hook_Factory(PlayerOperatedItemFrame, <PlayerOperatedItemFrameEventHook1, PlayerOperatedItemFrameEventHook2, PlayerOperatedItemFrameEventHook3>);

} // namespace ila::mc::inline world::inline actor::inline player