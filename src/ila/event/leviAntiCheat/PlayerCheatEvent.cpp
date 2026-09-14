#include "ila/event/leviAntiCheat/PlayerCheatEvent.h"
#include "ila/base/Gloabl.h"
#include "ila/event/leviAntiCheat/LeviAntiCheat.hpp"

namespace ila::lac {
void PlayerCheatEvent::serialize(CompoundTag& nbt) const {
    PlayerEvent::serialize(nbt);
    nbt["cheatType"] = magic_enum::enum_name(cheatType());
    nbt["type"]      = magic_enum::enum_name(type());
    nbt["duration"]  = duration();
    for (auto& [name, value] : extraData()) {
        std::visit([&](auto& value) -> void { nbt["extraData"][name] = value; }, value);
    }
}

CheckType const&  PlayerCheatEvent::cheatType() const { return mCheatType; }
ExtraInfo const&  PlayerCheatEvent::extraData() const { return mExtraData; }
int const&        PlayerCheatEvent::duration() const { return mDuration; }
PunishType const& PlayerCheatEvent::type() const { return mType; }

Event_Listener_Factory(PlayerCheat) {
    mListeners.emplace_back(
        LLEventBus.emplaceListener<::lac::punish::PlayerCheatEvent>([](::lac::punish::PlayerCheatEvent& event) -> void {
            // clang-format off
            auto ilaEvent = PlayerCheatEvent {
                event.self(),
                *std::bit_cast<CheckType const*>(event.mCheatType),
                *event.mExtraData,
                *event.mDuration,
                *std::bit_cast<PunishType const*>(event.mType)
            };
            LLEventBus.publish(ilaEvent);
            event.setCancelled(ilaEvent.isCancelled());
            // clang-format on
        })
    );
}

} // namespace ila::lac
