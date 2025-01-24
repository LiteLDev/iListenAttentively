#include "ila/event/pLand/PlayerAskCreateLandEvent.h"
#include "ila/base/Gloabl.h"

namespace land
{
void PlayerAskCreateLandAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["is3DLand"] = getIs3DLand();
}
bool    PlayerAskCreateLandAfterEvent::getIs3DLand() const { return mIs3DLand; }
} // namespace land