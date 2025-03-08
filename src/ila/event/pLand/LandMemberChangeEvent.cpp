#include "ila/event/pLand/LandMemberChangeEvent.h"
#include "ila/base/Gloabl.h"

namespace land
{
void LandMemberChangeBeforeEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["target"] = targetPlayer();
    nbt["landId"] = landId();
    nbt["isAdd"]  = isAdd();
}
std::string const& LandMemberChangeBeforeEvent::targetPlayer() const { return mTargetPlayer; }
uint64_t           LandMemberChangeBeforeEvent::landId() const { return mLandId; }
bool               LandMemberChangeBeforeEvent::isAdd() const { return mIsAdd; }

void LandMemberChangeAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["target"] = targetPlayer();
    nbt["landId"] = landId();
    nbt["isAdd"]  = isAdd();
}
std::string const& LandMemberChangeAfterEvent::targetPlayer() const { return mTargetPlayer; }
uint64_t           LandMemberChangeAfterEvent::landId() const { return mLandId; }
bool               LandMemberChangeAfterEvent::isAdd() const { return mIsAdd; }
} // namespace land