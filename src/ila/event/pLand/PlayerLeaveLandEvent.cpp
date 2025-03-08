#include "ila/event/pLand/PlayerLeaveLandEvent.h"
#include "ila/base/Gloabl.h"

namespace land
{
void PlayerLeaveLandEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["landId"] = landId();
}

uint64_t PlayerLeaveLandEvent::landId() const { return mLandId; }

} // namespace land