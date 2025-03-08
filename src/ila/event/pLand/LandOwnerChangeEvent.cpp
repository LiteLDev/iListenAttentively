#include "ila/event/pLand/LandOwnerChangeEvent.h"
#include "ila/base/Gloabl.h"

namespace land
{
void LandOwnerChangeBeforeEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["newOwner"] = ila::serializeRefObj(newOwner());
    nbt["landId"]   = landId();
}
Player&  LandOwnerChangeBeforeEvent::newOwner() const { return mNewOwner; }
uint64_t LandOwnerChangeBeforeEvent::landId() const { return mLandId; }

void LandOwnerChangeAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["newOwner"] = ila::serializeRefObj(newOwner());
    nbt["landId"]   = landId();
}
Player&  LandOwnerChangeAfterEvent::newOwner() const { return mNewOwner; }
uint64_t LandOwnerChangeAfterEvent::landId() const { return mLandId; }
} // namespace land