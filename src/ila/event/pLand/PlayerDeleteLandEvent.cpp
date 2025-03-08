#include "ila/event/pLand/PlayerDeleteLandEvent.h"
#include "ila/base/Gloabl.h"

namespace land
{
void PlayerDeleteLandBeforeEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["landId"]      = landId();
    nbt["refundPrice"] = refundPrice();
}
uint64_t   PlayerDeleteLandBeforeEvent::landId() const { return mLandId; }
int const& PlayerDeleteLandBeforeEvent::refundPrice() const { return mRefundPrice; }

void PlayerDeleteLandAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["landId"] = landId();
}
uint64_t PlayerDeleteLandAfterEvent::landId() const { return mLandId; }

} // namespace land