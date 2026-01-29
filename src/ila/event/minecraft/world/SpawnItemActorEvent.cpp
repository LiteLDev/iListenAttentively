#include "ila/event/minecraft/world/SpawnItemActorEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/deps/core/math/Vec3.h>
#include <mc/world/actor/item/ItemActor.h>
#include <mc/world/level/BedrockSpawner.h>

namespace ila::mc::inline world
{

void SpawnItemActorBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["pos"]       = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["dimId"]     = getDimensionName(blockSource());
    nbt["item"]      = serializeRefObj(mItem);
    nbt["spawner"]   = serializeRefObj(mSpawner);
    nbt["throwTime"] = mThrowTime;
}
void SpawnItemActorBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mPos.x     = nbt["pos"][0];
    mPos.y     = nbt["pos"][1];
    mPos.z     = nbt["pos"][2];
    mThrowTime = nbt["throwTime"];
}

void SpawnItemActorAfterEvent::serialize(CompoundTag& nbt) const
{
    WorldEvent::serialize(nbt);
    nbt["pos"]       = ListTag { mPos.x, mPos.y, mPos.z };
    nbt["dimId"]     = getDimensionName(blockSource());
    nbt["item"]      = serializeRefObj(mItem);
    nbt["spawner"]   = serializeRefObj(mSpawner);
    nbt["throwTime"] = mThrowTime;
    nbt["itemActor"] = serializeRefObj(mItemActor);
}

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