#include "ila/event/minecraft/world/level/block/actor/ChestPairWithEvent.h"
#include "ila/base/Gloabl.h"

namespace ila::mc::inline world::inline level::inline block::inline actor
{

void ChestPairWithBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["self"]  = serializeRefObj(self());
    nbt["chest"] = serializeRefObj(getChest());
    nbt["lead"]  = getLead();
}
void ChestPairWithBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    getLead() = nbt["lead"];
}
ChestBlockActor& ChestPairWithBeforeEvent::self() const
{
    return static_cast<ChestBlockActor&>(BlockActorEvent::self());
}
ChestBlockActor& ChestPairWithBeforeEvent::getChest() const { return mChest; }
bool&            ChestPairWithBeforeEvent::getLead() const { return mLead; }

void ChestPairWithAfterEvent::serialize(CompoundTag& nbt) const
{
    BlockActorEvent::serialize(nbt);
    nbt["self"]  = serializeRefObj(self());
    nbt["chest"] = serializeRefObj(getChest());
    nbt["lead"]  = getLead();
}
ChestBlockActor& ChestPairWithAfterEvent::self() const
{
    return static_cast<ChestBlockActor&>(BlockActorEvent::self());
}
ChestBlockActor& ChestPairWithAfterEvent::getChest() const { return mChest; }
bool const&      ChestPairWithAfterEvent::getLead() const { return mLead; }

LL_TYPE_INSTANCE_HOOK(
    ChestPairWithEventHook,
    HookPriority::Normal,
    ChestBlockActor,
    &ChestBlockActor::pairWith,
    void,
    ChestBlockActor* pChest,
    bool             pLead
)
{
    if (pChest == nullptr) { return origin(pChest, pLead); }
    auto beforeEvent = ChestPairWithBeforeEvent(*this, *pChest, pLead);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return; }
    origin(pChest, pLead);
    LLEventBus.publish(ChestPairWithAfterEvent(*this, *pChest, pLead));
}

Event_Hook_Factory(ChestPairWith, <ChestPairWithEventHook>);

} // namespace ila::mc::inline world::inline level::inline block::inline actor