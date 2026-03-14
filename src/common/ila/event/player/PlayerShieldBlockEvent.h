#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/player/PlayerEvent.h>

namespace ila::player {

class PlayerShieldBlockEvent : public ll::event::PlayerEvent {
private:
    ActorDamageSource const& mSource;
    float                    mDamage;

public:
    constexpr explicit PlayerShieldBlockEvent(Player& player, ActorDamageSource const& source, float damage)
    : PlayerEvent(player),
      mSource(source),
      mDamage(damage) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;

public:
    ActorDamageSource const& source() const { return mSource; }
};

/** @warning This event is not available on the client side. */
class PlayerShieldBlockingEvent final : public ll::event::Cancellable<PlayerShieldBlockEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class PlayerShieldBlockedEvent final : public PlayerShieldBlockEvent {
public:
    using PlayerShieldBlockEvent::PlayerShieldBlockEvent;
};

} // namespace ila::player