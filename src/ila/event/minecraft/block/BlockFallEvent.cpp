#include "ila/event/minecraft/block/BlockFallEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/entity/ActorEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/deps/core/math/Vec2.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/ecs/gamerefs_entity/GameRefsEntity.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorDefinitionIdentifier.h>
#include <mc/world/actor/ActorFactory.h>
#include <mc/world/actor/ActorType.h>
#include <mc/world/actor/item/FallingBlockActor.h>
#include <mc/world/level/ActorBlockSyncMessage.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/ActorChangeContext.h>
#include <mc/world/level/block/BedrockBlockNames.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/BlockChangeContext.h>
#include <mc/world/level/block/FallingBlock.h>
#include <mc/world/level/block/registry/BlockTypeRegistry.h>
#include <utility>

namespace ila::mc::inline block
{

void BlockFallBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]      = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["oldBlock"] = serializeRefObj(mOldBlock);
    nbt["creative"] = mCreative;
    nbt["dimId"]    = getDimensionName(blockSource());
}
void BlockFallBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mCreative = nbt["creative"];
}

void BlockFallAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["pos"]  = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["self"] = serializeRefObj(self());
}
FallingBlockActor& BlockFallAfterEvent::self() const
{
    return static_cast<FallingBlockActor&>(ActorEvent::self());
}

LL_TYPE_INSTANCE_HOOK(
    BlockFallEventHook,
    HookPriority::Normal,
    FallingBlock,
    &FallingBlock::$startFalling,
    void,
    BlockSource&    pRegion,
    BlockPos const& pPos,
    Block const&    pOldBlock,
    bool            pCreative
)
{
    auto beforeEvent = BlockFallBeforeEvent(pRegion, pPos, pOldBlock, pCreative);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    auto actorContext = pRegion.getLevel().getActorFactory().createSpawnedActor(
        ActorDefinitionIdentifier(ActorType::FallingBlock, ""),
        nullptr,
        pPos.center(),
        0
    );
    auto actor = actorContext.tryUnwrap().transform([](Actor& actor) -> FallingBlockActor& {
        return static_cast<FallingBlockActor&>(actor);
    });
    if (!actor || actor->mRemoved) return;
    actor->setFallingBlock(pOldBlock, pCreative);
    actor->mLevel = &pRegion.getLevel();
    auto syncMsg =
        ActorBlockSyncMessage { actor->getOrCreateUniqueID(), ActorBlockSyncMessage::MessageId::None };
    BlockChangeContext context { false };
    context.mContextSource = { ActorChangeContext { actor } };
    pRegion.setBlock(
        pPos,
        BlockTypeRegistry::get().getDefaultBlockState(BedrockBlockNames::Air()),
        3,
        &syncMsg,
        context
    );
    static_cast<FallingBlock const&>(pOldBlock.getBlockType())
        ._tickBlocksAround2D(pRegion, pPos.add({ 0, 1, 0 }), pOldBlock);
    static_cast<FallingBlock const&>(pOldBlock.getBlockType())
        ._tickBlocksAround2D(pRegion, pPos.add({ 0, -1, 0 }), pOldBlock);
    pRegion.getLevel().addEntity(pRegion, std::move(actorContext));
    LLEventBus.publish(BlockFallAfterEvent(actor, pPos));
}

Event_Hook_Factory(BlockFall, <BlockFallEventHook>);

} // namespace ila::mc::inline block