#include "ila/event/pLand/LandMemberChangeEvent.h"
#include "ila/base/Gloabl.h"

namespace land
{
void LandMemberChangeBeforeEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["target"] = getTargetPlayer();
    nbt["landId"] = getLandId();
    nbt["isAdd"]  = getIsAdd();
}
std::string const& LandMemberChangeBeforeEvent::getTargetPlayer() const { return mTargetPlayer; }
uint64_t           LandMemberChangeBeforeEvent::getLandId() const { return mLandId; }
bool               LandMemberChangeBeforeEvent::getIsAdd() const { return mIsAdd; }

void LandMemberChangeAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["target"] = getTargetPlayer();
    nbt["landId"] = getLandId();
    nbt["isAdd"]  = getIsAdd();
}
std::string const& LandMemberChangeAfterEvent::getTargetPlayer() const { return mTargetPlayer; }
uint64_t           LandMemberChangeAfterEvent::getLandId() const { return mLandId; }
bool               LandMemberChangeAfterEvent::getIsAdd() const { return mIsAdd; }
} // namespace land