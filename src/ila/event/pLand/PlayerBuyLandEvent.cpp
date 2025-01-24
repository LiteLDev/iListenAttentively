#include "ila/event/pLand/PlayerBuyLandEvent.h"
#include "ila/base/Gloabl.h"

namespace land
{
void PlayerBuyLandBeforeEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["landSelectorData"] = ila::serializePtrObj(getLandSelectorData());
    nbt["price"]            = getPrice();
}
void PlayerBuyLandBeforeEvent::deserialize(CompoundTag const& nbt)
{
    PlayerEvent::deserialize(nbt);
    getPrice() = nbt["price"];
}
LandSelectorData* PlayerBuyLandBeforeEvent::getLandSelectorData() const { return mLandSelectorData; }
int&              PlayerBuyLandBeforeEvent::getPrice() const { return mPrice; }

void PlayerBuyLandAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["landData"] = ila::serializePtrObj(getLandData().get());
}
std::shared_ptr<class LandData> PlayerBuyLandAfterEvent::getLandData() const { return mLandData; }
} // namespace land