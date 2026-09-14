#include "ila/event/leviAntiCheat/PlayerBanWaveEvent.h"
#include "ila/base/Gloabl.h"
#include "ila/event/leviAntiCheat/LeviAntiCheat.hpp"

namespace ila::lac {
void PlayerBanWaveEvent::serialize(CompoundTag& nbt) const {
    PlayerEvent::serialize(nbt);
    nbt["type"] = magic_enum::enum_name(type());
}

BanWaveType PlayerBanWaveEvent::type() const { return mType; }

Event_Listener_Factory(PlayerBanWave) {
    mListeners.emplace_back(LLEventBus.emplaceListener<::lac::punish::PlayerBanWaveEvent>(
        [](::lac::punish::PlayerBanWaveEvent& event) -> void {
            // clang-format off
            auto ilaEvent = PlayerBanWaveEvent {
                event.self(),
                std::bit_cast<BanWaveType>(event.mType)
            };
            LLEventBus.publish(ilaEvent);
            event.setCancelled(ilaEvent.isCancelled());
            // clang-format on
        }
    ));
}
} // namespace ila::lac
