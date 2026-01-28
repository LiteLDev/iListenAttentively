#include "ila/event/minecraft/actor/ActorChangeDimensionEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/deps/core/math/Vec3.h>
#include <mc/world/level/ActorDimensionTransferManager.h>
#include <mc/world/level/dimension/Dimension.h>

namespace ila::mc::inline world::inline actor
{

void ActorChangeDimensionBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["fromDimensionId"] = getDimensionName(fromDimensionId());
    nbt["toDimensionId"]   = getDimensionName(toDimensionId());
}
void ActorChangeDimensionBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    toDimensionId() = getDimensionId(nbt["toDimensionId"]);
}
DimensionType const& ActorChangeDimensionBeforeEvent::fromDimensionId() const { return mFromDimensionId; };
DimensionType&       ActorChangeDimensionBeforeEvent::toDimensionId() const { return mToDimensionId; };

void ActorChangeDimensionAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["fromDimensionId"] = fromDimensionId().id;
    nbt["fromPos"]         = ListTag { getFromPos().x, getFromPos().y, getFromPos().z };
    nbt["toDimensionId"]   = toDimensionId().id;
}
DimensionType const& ActorChangeDimensionAfterEvent::fromDimensionId() const { return mFromDimensionId; };
Vec3 const&          ActorChangeDimensionAfterEvent::getFromPos() const { return mFromPos; };
DimensionType const& ActorChangeDimensionAfterEvent::toDimensionId() const { return mToDimensionId; };

LL_TYPE_INSTANCE_HOOK(
    ActorChangeDimensionEventHook1,
    HookPriority::Normal,
    ActorDimensionTransferManager,
    &ActorDimensionTransferManager::canChangeDimension,
    bool,
    Actor const&  pActor,
    DimensionType pToId
)
{
    auto result = origin(pActor, pToId);
    if (!result) { return false; }
    auto const formId      = *pActor.mDimension->lock()->mId;
    auto       beforeEvent = ActorChangeDimensionBeforeEvent(const_cast<Actor&>(pActor), formId, pToId);
    LLEventBus.publish(beforeEvent);
    return !beforeEvent.isCancelled();
}

LL_TYPE_INSTANCE_HOOK(
    ActorChangeDimensionEventHook2,
    HookPriority::Normal,
    ActorDimensionTransferManager,
    &ActorDimensionTransferManager::actorChangeDimension,
    void,
    Actor&                     pActor,
    DimensionType              pToId,
    std::optional<Vec3> const& actorPosition
)
{
    auto const fromId  = *pActor.mDimension->lock()->mId;
    auto const fromPos = pActor.getPosition();
    origin(pActor, pToId, actorPosition);
    if (fromId == pToId) { return; }
    LLEventBus.publish(ActorChangeDimensionAfterEvent(const_cast<Actor&>(pActor), fromId, fromPos, pToId));
}

Event_Hook_Factory(ActorChangeDimension, <ActorChangeDimensionEventHook1, ActorChangeDimensionEventHook2>);

} // namespace ila::mc::inline world::inline actor