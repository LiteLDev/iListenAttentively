#include "ila/event/minecraft/world/level/block/BlockFallEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/deps/core/math/Vec2.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/ecs/gamerefs_entity/GameRefsEntity.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/world/actor/ActorDefinitionIdentifier.h>
#include <mc/world/actor/ActorFactory.h>
#include <mc/world/actor/item/FallingBlockActor.h>
#include <mc/world/level/ActorBlockSyncMessage.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/BedrockBlockNames.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/FallingBlock.h>
#include <mc/world/level/block/registry/BlockTypeRegistry.h>

namespace ila::mc::inline world::inline level::inline block
{

void BlockFallBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]      = ListTag { pos().x, pos().y, pos().z };
    nbt["oldBlock"] = serializeRefObj(oldBlock());
    nbt["creative"] = creative();
    nbt["dimId"]    = getDimensionName(blockSource());
}
void BlockFallBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    creative() = nbt["creative"];
}
BlockPos const& BlockFallBeforeEvent::pos() const { return mPos; }
Block const&    BlockFallBeforeEvent::oldBlock() const { return mOldBlock; }
bool&           BlockFallBeforeEvent::creative() const { return mCreative; }

void BlockFallAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["pos"]    = ListTag { pos().x, pos().y, pos().z };
    nbt["self"] = serializeRefObj(self());
}
BlockPos const&    BlockFallAfterEvent::pos() const { return mPos; }
FallingBlockActor& BlockFallAfterEvent::self() const
{
    return static_cast<FallingBlockActor&>(ActorEvent::self());
}

LL_AUTO_TYPE_INSTANCE_HOOK(
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
        ActorDefinitionIdentifier(ActorType::FallingBlock),
        nullptr,
        pPos.center(),
        Vec2::ZERO()
    );
    auto actor = actorContext.tryUnwrap().transform([](Actor& actor) -> FallingBlockActor& {
        return static_cast<FallingBlockActor&>(actor);
    });
    if (!actor || actor->mRemoved) return;
    actor->setFallingBlock(pOldBlock, pCreative);
    actor->mLevel = &pRegion.getLevel();
    auto syncMsg =
        ActorBlockSyncMessage { actor->getOrCreateUniqueID(), ActorBlockSyncMessage::MessageId::None };
    pRegion.setBlock(
        pPos,
        BlockTypeRegistry::getDefaultBlockState(BedrockBlockNames::Air()),
        3,
        &syncMsg,
        nullptr
    );
    static_cast<FallingBlock const&>(pOldBlock.getLegacyBlock())
        ._tickBlocksAround2D(pRegion, pPos.add({ 0, 1, 0 }), pOldBlock);
    static_cast<FallingBlock const&>(pOldBlock.getLegacyBlock())
        ._tickBlocksAround2D(pRegion, pPos.add({ 0, -1, 0 }), pOldBlock);
    pRegion.getLevel().addEntity(pRegion, std::move(actorContext));
    LLEventBus.publish(BlockFallAfterEvent(actor, pPos));
}

Event_Hook_Factory(BlockFall, <BlockFallEventHook>);

} // namespace ila::mc::inline world::inline level::inline block