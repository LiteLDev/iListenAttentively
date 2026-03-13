#include "ila/event/actor/player/PlayerChangeDimensionEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/player/PlayerEvent.h>
#include <ll/api/memory/Hook.h>
#include <magic_enum.hpp>
#include <mc/_HeaderOutputPredefine.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/ChangeDimensionRequest.h>
#include <mc/world/level/PlayerDimensionTransferer.h>

namespace ila::mc::inline actor::inline player
{

void PlayerChangeDimensionBeforeEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["state"]           = magic_enum::enum_name(mChangeDimensionRequest.mState);
    nbt["fromDimensionId"] = mChangeDimensionRequest.mFromDimensionId->id;
    nbt["toDimensionId"]   = mChangeDimensionRequest.mToDimensionId->id;
    nbt["fromLocation"]    = ListTag { mChangeDimensionRequest.mFromLocation->x,
                                    mChangeDimensionRequest.mFromLocation->y,
                                    mChangeDimensionRequest.mFromLocation->z };
    nbt["toLocation"]      = ListTag { mChangeDimensionRequest.mToLocation->x,
                                  mChangeDimensionRequest.mToLocation->y,
                                  mChangeDimensionRequest.mToLocation->z };
    nbt["usePortal"]       = mChangeDimensionRequest.mUsePortal;
    nbt["respawn"]         = mChangeDimensionRequest.mRespawn;
    if (mChangeDimensionRequest.mAgentTag) { nbt["agentTag"] = *mChangeDimensionRequest.mAgentTag; }
}

void PlayerChangeDimensionAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["state"]           = magic_enum::enum_name(mChangeDimensionRequest.mState);
    nbt["fromDimensionId"] = mChangeDimensionRequest.mFromDimensionId->id;
    nbt["toDimensionId"]   = mChangeDimensionRequest.mToDimensionId->id;
    nbt["fromLocation"]    = ListTag { mChangeDimensionRequest.mFromLocation->x,
                                    mChangeDimensionRequest.mFromLocation->y,
                                    mChangeDimensionRequest.mFromLocation->z };
    nbt["toLocation"]      = ListTag { mChangeDimensionRequest.mToLocation->x,
                                  mChangeDimensionRequest.mToLocation->y,
                                  mChangeDimensionRequest.mToLocation->z };
    nbt["usePortal"]       = mChangeDimensionRequest.mUsePortal;
    nbt["respawn"]         = mChangeDimensionRequest.mRespawn;
    if (mChangeDimensionRequest.mAgentTag) { nbt["agentTag"] = *mChangeDimensionRequest.mAgentTag; }
}

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
} // namespace ila::mc::inline actor::inline player