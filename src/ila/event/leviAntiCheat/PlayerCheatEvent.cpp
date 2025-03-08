#include "ila/event/leviAntiCheat/PlayerCheatEvent.h"
#include "ila/base/Gloabl.h"

namespace lac::punish
{
void PlayerCheatEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["cheatType"] = magic_enum::enum_name(cheatType());
    nbt["type"]      = magic_enum::enum_name(type());
    nbt["duration"]  = duration();
    for (auto& [name, value] : extraData())
    {
        std::visit([&](auto& value) -> void { nbt["extraData"][name] = value; }, value);
    }
}

CheckType const&  PlayerCheatEvent::cheatType() const { return mCheatType; }
ExtraInfo const&  PlayerCheatEvent::extraData() const { return mExtraData; }
int const&        PlayerCheatEvent::duration() const { return mDuration; }
PunishType const& PlayerCheatEvent::type() const { return mType; }
} // namespace lac::punish