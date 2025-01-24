#include "ila/event/pLand/PlayerEnterLandEvent.h"
#include "ila/base/Gloabl.h"

namespace land
{
void PlayerEnterLandEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["landId"] = getLandId();
}

uint64_t PlayerEnterLandEvent::getLandId() const { return mLandId; }

} // namespace land