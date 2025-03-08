#include "ila/event/pLand/PlayerEnterLandEvent.h"
#include "ila/base/Gloabl.h"

namespace land
{
void PlayerEnterLandEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["landId"] = landId();
}

uint64_t PlayerEnterLandEvent::landId() const { return mLandId; }

} // namespace land