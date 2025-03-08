#include "ila/event/pLand/LandRangeChangeEvent.h"
#include "ila/base/Gloabl.h"

namespace land
{
void LandRangeChangeBeforeEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["landData"]    = ila::serializePtrObj(landData().get());
    nbt["newRange"]    = ila::serializeRefObj(newRange());
    nbt["needPay"]     = needPay();
    nbt["refundPrice"] = refundPrice();
}
std::shared_ptr<class LandData> const& LandRangeChangeBeforeEvent::landData() const { return mLandData; }
class LandPos const&                   LandRangeChangeBeforeEvent::newRange() const { return mNewRange; }
int const&                             LandRangeChangeBeforeEvent::needPay() const { return mNeedPay; }
int const& LandRangeChangeBeforeEvent::refundPrice() const { return mRefundPrice; }

void LandRangeChangeAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["landData"]    = ila::serializePtrObj(landData().get());
    nbt["newRange"]    = ila::serializeRefObj(newRange());
    nbt["needPay"]     = needPay();
    nbt["refundPrice"] = refundPrice();
}
std::shared_ptr<class LandData> const& LandRangeChangeAfterEvent::landData() const { return mLandData; }
class LandPos const&                   LandRangeChangeAfterEvent::newRange() const { return mNewRange; }
int const&                             LandRangeChangeAfterEvent::needPay() const { return mNeedPay; }
int const& LandRangeChangeAfterEvent::refundPrice() const { return mRefundPrice; }
} // namespace land