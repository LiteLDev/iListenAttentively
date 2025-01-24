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
        if (std::holds_alternative<std::string>(value))
        {
            nbt["extraData"][name] = std::get<std::string>(value);
        }
        else if (std::holds_alternative<int>(value)) { nbt["extraData"][name] = std::get<int>(value); }
        else if (std::holds_alternative<ullong>(value)) { nbt["extraData"][name] = std::get<ullong>(value); }
        else if (std::holds_alternative<llong>(value)) { nbt["extraData"][name] = std::get<llong>(value); }
        else if (std::holds_alternative<std::string_view>(value))
        {
            nbt["extraData"][name] = std::get<std::string_view>(value);
        }
        else if (std::holds_alternative<float>(value)) { nbt["extraData"][name] = std::get<float>(value); }
        else { throw std::runtime_error("Invalid extra data type"); }
    }
}

CheckType const&  PlayerCheatEvent::getCheatType() const { return mCheatType; }
ExtraInfo const&  PlayerCheatEvent::getExtraData() const { return mExtraData; }
int const&        PlayerCheatEvent::getDuration() const { return mDuration; }
PunishType const& PlayerCheatEvent::getType() const { return mType; }
} // namespace lac::punish