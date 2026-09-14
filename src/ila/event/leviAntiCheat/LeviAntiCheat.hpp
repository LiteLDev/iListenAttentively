#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/player/PlayerEvent.h>
#include <mc/platform/UUID.h>

namespace lac::punish {

enum class BanWaveType { Kick, Ban };

enum class CheckType {
    IllegalMovement,
    Timer,
    Spam,
    InvalidFilterString,
    IllegalBreaking,
    FakeName,
    SpawnXpOrbs,
    Toolbox,
    AutoClick,
    Reach,
    InvalidNbtItem,
    InvalidAnvilEnchant,
    AutoOffhand,
    BanItem,
    InvalidEnchantLevel,
    InvalidGetItem,
    InvalidStackItem,
    MessageTooLong,
    CommandSpam,
    XpHack,
    NoPacket,
    BadPacket,
};

enum class PunishType { Warning = 0, Mute = 1, Kick = 2, Ban = 3, Cancel = 4, None = 5 };

using ExtraInfo =
    std::unordered_map<std::string, std::variant<std::string, int, ullong, llong, std::string_view, float>>;

class SusClientEvent final : public ll::event::Cancellable<ll::event::Event> {
public:
    mce::UUID const*        mUuid;
    std::string_view const* mName;
    std::string_view const* mIp;
};

class PlayerBanWaveEvent final : public ll::event::Cancellable<ll::event::PlayerEvent> {
public:
    BanWaveType mType;
};

class PlayerCheatEvent final : public ll::event::Cancellable<ll::event::PlayerEvent> {
public:
    CheckType const*  mCheatType;
    ExtraInfo const*  mExtraData;
    int const*        mDuration;
    PunishType const* mType;
};

} // namespace lac::punish
