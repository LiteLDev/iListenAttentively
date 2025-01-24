#include "ila/event/pLand/LandRangeChangeEvent.h"
#include "ila/base/Gloabl.h"

namespace land
{
void LandRangeChangeBeforeEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["landData"]    = ila::serializePtrObj(getLandData().get());
    nbt["newRange"]    = ila::serializeRefObj(getNewRange());
    nbt["needPay"]     = getNeedPay();
    nbt["refundPrice"] = getRefundPrice();
}
std::shared_ptr<class LandData> const& LandRangeChangeBeforeEvent::getLandData() const { return mLandData; }
class LandPos const&                   LandRangeChangeBeforeEvent::getNewRange() const { return mNewRange; }
int const&                             LandRangeChangeBeforeEvent::getNeedPay() const { return mNeedPay; }
int const& LandRangeChangeBeforeEvent::getRefundPrice() const { return mRefundPrice; }

void LandRangeChangeAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["landData"]    = ila::serializePtrObj(getLandData().get());
    nbt["newRange"]    = ila::serializeRefObj(getNewRange());
    nbt["needPay"]     = getNeedPay();
    nbt["refundPrice"] = getRefundPrice();
}
std::shared_ptr<class LandData> const& LandRangeChangeAfterEvent::getLandData() const { return mLandData; }
class LandPos const&                   LandRangeChangeAfterEvent::getNewRange() const { return mNewRange; }
int const&                             LandRangeChangeAfterEvent::getNeedPay() const { return mNeedPay; }
int const& LandRangeChangeAfterEvent::getRefundPrice() const { return mRefundPrice; }
} // namespace land