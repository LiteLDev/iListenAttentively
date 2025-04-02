#include "ila/event/minecraft/world/SpawnItemActorEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/level/BedrockSpawner.h>

namespace ila::mc::inline world
{

void SpawnItemActorBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]       = ListTag { pos().x, pos().y, pos().z };
    nbt["dimId"]     = getDimensionName(blockSource());
    nbt["item"]      = serializeRefObj(item());
    nbt["spawner"]   = serializeRefObj(spawner());
    nbt["throwTime"] = throwTime();
}
void SpawnItemActorBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    pos().x     = nbt["pos"][0];
    pos().y     = nbt["pos"][1];
    pos().z     = nbt["pos"][2];
    throwTime() = nbt["throwTime"];
}
Vec3&      SpawnItemActorBeforeEvent::pos() const { return mPos; }
ItemStack& SpawnItemActorBeforeEvent::item() const { return mItem; }
Actor*&    SpawnItemActorBeforeEvent::spawner() const { return mSpawner; }
int&       SpawnItemActorBeforeEvent::throwTime() const { return mThrowTime; }

void SpawnItemActorAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]       = ListTag { pos().x, pos().y, pos().z };
    nbt["dimId"]     = getDimensionName(blockSource());
    nbt["item"]      = serializeRefObj(item());
    nbt["spawner"]   = serializeRefObj(spawner());
    nbt["throwTime"] = throwTime();
    nbt["itemActor"] = serializeRefObj(itemActor());
}
Vec3 const&      SpawnItemActorAfterEvent::pos() const { return mPos; }
ItemStack const& SpawnItemActorAfterEvent::item() const { return mItem; }
Actor* const&    SpawnItemActorAfterEvent::spawner() const { return mSpawner; }
int const&       SpawnItemActorAfterEvent::throwTime() const { return mThrowTime; }
ItemActor&       SpawnItemActorAfterEvent::itemActor() const { return mItemActor; }

LL_TYPE_INSTANCE_HOOK(
    SpawnItemActorEventHook,
    HookPriority::Normal,
    BedrockSpawner,
    &BedrockSpawner::$spawnItem,
    ItemActor*,
    BlockSource&     pRegion,
    ItemStack const& pItem,
    Actor*           pSpawner,
    Vec3 const&      pPos,
    int              pThrowTime
)
{
    auto beforeEvent = SpawnItemActorBeforeEvent(
        pRegion,
        const_cast<Vec3&>(pPos),
        const_cast<ItemStack&>(pItem),
        pSpawner,
        pThrowTime
    );
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return nullptr; }
    auto* result = origin(pRegion, pItem, pSpawner, pPos, pThrowTime);
    if (result != nullptr)
    {
        LLEventBus.publish(SpawnItemActorAfterEvent(pRegion, pPos, pItem, pSpawner, pThrowTime, *result));
    }
    return result;
}

Event_Hook_Factory(SpawnItemActor, <SpawnItemActorEventHook>);

} // namespace ila::mc::inline world