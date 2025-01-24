#include "ila/event/pLand/PlayerLeaveLandEvent.h"
#include "ila/base/Gloabl.h"

namespace land
{
void PlayerLeaveLandEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["landId"] = getLandId();
}

uint64_t PlayerLeaveLandEvent::getLandId() const { return mLandId; }

} // namespace land