#include "ila/event/minecraft/world/actor/player/PlayerChangeDimensionEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/deps/core/math/Vec3.h>
#include <mc/world/level/ChangeDimensionRequest.h>
#include <mc/world/level/PlayerDimensionTransferer.h>

namespace ila::mc::inline world::inline actor::inline player
{

void PlayerChangeDimensionBeforeEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["state"]           = magic_enum::enum_name(changeDimensionRequest().mState);
    nbt["fromDimensionId"] = changeDimensionRequest().mFromDimensionId->id;
    nbt["toDimensionId"]   = changeDimensionRequest().mToDimensionId->id;
    nbt["fromLocation"]    = ListTag { changeDimensionRequest().mFromLocation->x,
                                    changeDimensionRequest().mFromLocation->y,
                                    changeDimensionRequest().mFromLocation->z };
    nbt["toLocation"]      = ListTag { changeDimensionRequest().mToLocation->x,
                                  changeDimensionRequest().mToLocation->y,
                                  changeDimensionRequest().mToLocation->z };
    nbt["usePortal"]       = changeDimensionRequest().mUsePortal;
    nbt["respawn"]         = changeDimensionRequest().mRespawn;
    if (changeDimensionRequest().mAgentTag) { nbt["agentTag"] = *changeDimensionRequest().mAgentTag; }
}
ChangeDimensionRequest const& PlayerChangeDimensionBeforeEvent::changeDimensionRequest() const
{
    return mChangeDimensionRequest;
}
Dimension const& PlayerChangeDimensionBeforeEvent::dimension() const { return mDimension; }

void PlayerChangeDimensionAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["state"]           = magic_enum::enum_name(changeDimensionRequest().mState);
    nbt["fromDimensionId"] = changeDimensionRequest().mFromDimensionId->id;
    nbt["toDimensionId"]   = changeDimensionRequest().mToDimensionId->id;
    nbt["fromLocation"]    = ListTag { changeDimensionRequest().mFromLocation->x,
                                    changeDimensionRequest().mFromLocation->y,
                                    changeDimensionRequest().mFromLocation->z };
    nbt["toLocation"]      = ListTag { changeDimensionRequest().mToLocation->x,
                                  changeDimensionRequest().mToLocation->y,
                                  changeDimensionRequest().mToLocation->z };
    nbt["usePortal"]       = changeDimensionRequest().mUsePortal;
    nbt["respawn"]         = changeDimensionRequest().mRespawn;
    if (changeDimensionRequest().mAgentTag) { nbt["agentTag"] = *changeDimensionRequest().mAgentTag; }
}
ChangeDimensionRequest const& PlayerChangeDimensionAfterEvent::changeDimensionRequest() const
{
    return mChangeDimensionRequest;
}
Dimension const& PlayerChangeDimensionAfterEvent::dimension() const { return mDimension; }

LL_TYPE_INSTANCE_HOOK(
    PlayerChangeDimensionEventHook,
    HookPriority::Normal,
    PlayerDimensionTransferer,
    &PlayerDimensionTransferer::$playerPrepareRegion,
    void,
    Player&                       pPlayer,
    ChangeDimensionRequest const& pChangeDimensionRequest,
    Dimension const&              pDimension
)
{
    LLEventBus.publish(PlayerChangeDimensionBeforeEvent(pPlayer, pChangeDimensionRequest, pDimension));
    origin(pPlayer, pChangeDimensionRequest, pDimension);
    LLEventBus.publish(PlayerChangeDimensionAfterEvent(pPlayer, pChangeDimensionRequest, pDimension));
}

Event_Hook_Factory(PlayerChangeDimension, <PlayerChangeDimensionEventHook>);
} // namespace ila::mc::inline world::inline actor::inline player