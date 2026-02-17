#include "ila/event/minecraft/actor/ActorChangeDimensionEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/entity/ActorEvent.h>
#include <ll/api/memory/Hook.h>
#include <mc/_HeaderOutputPredefine.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/level/ActorDimensionTransferManager.h>
#include <mc/world/level/dimension/Dimension.h>
#include <optional>

namespace ila::mc::inline actor
{

void ActorChangeDimensionBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["fromDimensionId"] = getDimensionName(mFromDimensionId);
    nbt["toDimensionId"]   = getDimensionName(mToDimensionId);
}
void ActorChangeDimensionBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mToDimensionId = getDimensionId(nbt["toDimensionId"]);
};;

void ActorChangeDimensionAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["fromDimensionId"] = mFromDimensionId.id;
    nbt["fromPos"]         = ListTag { mFromPos.x, mFromPos.y, mFromPos.z };
    nbt["toDimensionId"]   = mToDimensionId.id;
};;;

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

} // namespace ila::mc::inline actor