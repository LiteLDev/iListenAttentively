#include "ila/event/pLand/PlayerAskCreateLandEvent.h"
#include "ila/base/Gloabl.h"

namespace land
{
void PlayerAskCreateLandAfterEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["is3DLand"] = is3DLand();
}
bool    PlayerAskCreateLandAfterEvent::is3DLand() const { return mIs3DLand; }
} // namespace land