#include "ila/event/pLand/LandOwnerChangeEvent.h"
#include "ila/base/Gloabl.h"

namespace land
{
void LandOwnerChangeBeforeEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["newOwner"] = ila::serializeRefObj(getNewOwner());
    nbt["landId"]   = getLandId();
}
Player&  LandOwnerChangeBeforeEvent::getNewOwner() const { return mNewOwner; }
uint64_t LandOwnerChangeBeforeEvent::getLandId() const { return mLandId; }

void LandOwnerChangeAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["newOwner"] = ila::serializeRefObj(getNewOwner());
    nbt["landId"]   = getLandId();
}
Player&  LandOwnerChangeAfterEvent::getNewOwner() const { return mNewOwner; }
uint64_t LandOwnerChangeAfterEvent::getLandId() const { return mLandId; }
} // namespace land