#include "ila/event/pLand/PlayerBuyLandEvent.h"
#include "ila/base/Gloabl.h"

namespace land
{
void PlayerBuyLandBeforeEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["is3DLand"] = ila::serializePtrObj(landSelectorData());
    nbt["price"]            = price();
}
void PlayerBuyLandBeforeEvent::deserialize(CompoundTag const& nbt)
{
    PlayerEvent::deserialize(nbt);
    price() = nbt["price"];
}
LandSelectorData* PlayerBuyLandBeforeEvent::landSelectorData() const { return mLandSelectorData; }
int&              PlayerBuyLandBeforeEvent::price() const { return mPrice; }

void PlayerBuyLandAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["landData"] = ila::serializePtrObj(landData().get());
}
std::shared_ptr<class LandData> PlayerBuyLandAfterEvent::landData() const { return mLandData; }
} // namespace land