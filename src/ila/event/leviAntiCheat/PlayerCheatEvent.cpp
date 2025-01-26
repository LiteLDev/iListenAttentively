#include "ila/event/leviAntiCheat/PlayerCheatEvent.h"
#include "ila/base/Gloabl.h"

namespace lac::punish
{
void PlayerCheatEvent::serialize(CompoundTag& nbt) const
{
    PlayerEvent::serialize(nbt);
    nbt["cheatType"] = magic_enum::enum_name(getCheatType());
    nbt["type"]      = magic_enum::enum_name(getType());
    nbt["duration"]  = getDuration();
    for (auto& [name, value] : getExtraData())
    {
        std::visit([&](auto& value) -> void { nbt["extraData"][name] = value; }, value);
    }
}

CheckType const&  PlayerCheatEvent::getCheatType() const { return mCheatType; }
ExtraInfo const&  PlayerCheatEvent::getExtraData() const { return mExtraData; }
int const&        PlayerCheatEvent::getDuration() const { return mDuration; }
PunishType const& PlayerCheatEvent::getType() const { return mType; }
} // namespace lac::punish