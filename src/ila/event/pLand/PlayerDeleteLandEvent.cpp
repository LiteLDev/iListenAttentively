#include "ila/event/pLand/PlayerDeleteLandEvent.h"
#include "ila/base/Gloabl.h"

namespace land
{
void PlayerDeleteLandBeforeEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["landId"]      = getLandId();
    nbt["refundPrice"] = getRefundPrice();
}
uint64_t   PlayerDeleteLandBeforeEvent::getLandId() const { return mLandId; }
int const& PlayerDeleteLandBeforeEvent::getRefundPrice() const { return mRefundPrice; }

void PlayerDeleteLandAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["landId"] = getLandId();
}
uint64_t PlayerDeleteLandAfterEvent::getLandId() const { return mLandId; }

} // namespace land