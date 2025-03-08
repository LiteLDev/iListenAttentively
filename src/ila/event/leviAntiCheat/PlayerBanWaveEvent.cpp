#include "ila/event/leviAntiCheat/PlayerBanWaveEvent.h"
#include "ila/base/Gloabl.h"

namespace lac::punish
{
void PlayerBanWaveEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["type"] = magic_enum::enum_name(type());
}

BanWaveType PlayerBanWaveEvent::type() const { return mType; }
} // namespace lac::punish