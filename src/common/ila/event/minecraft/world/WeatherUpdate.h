#pragma once
#include "ila/base/Macro.h"
#include <ll/api/chrono/GameChrono.h>
#include <ll/api/event/Cancellable.h>

// #include <ll/api/event/world/LevelEvent.h>

namespace ila::mc::inline world {

class WeatherUpdateEvent : public ll::event::Event {
public:
    enum class Type {
        Clear   = 0x0,
        Rain    = 0x1,
        Thunder = 0x2,
    };

protected:
    std::pair<Type, ll::chrono::ticks> mPrevState;
    std::pair<Type, ll::chrono::ticks> mNextState;

public:
    constexpr explicit WeatherUpdateEvent(
        std::pair<Type, ll::chrono::ticks> prevState,
        std::pair<Type, ll::chrono::ticks> nextState
    )
    : Event(),
      mPrevState(prevState),
      mNextState(nextState) {}

public:
    ILAAPI void serialize(CompoundTag& nbt) const override;

public:
    Type              prevType() const { return mPrevState.first; }
    ll::chrono::ticks prevDuration() const { return mPrevState.second; }
    Type              nextType() const { return mNextState.first; }
    ll::chrono::ticks nextDuration() const { return mNextState.second; }
};

class WeatherUpdatingEvent final : public ll::event::Cancellable<WeatherUpdateEvent> {
public:
    using Cancellable::Cancellable;

public:
    ILAAPI void deserialize(CompoundTag const& nbt) override;

public:
    void setClear() { mNextState = {Type::Clear, ll::chrono::ticks::zero()}; }
    void setRain(ll::chrono::ticks time) {
        if (time <= ll::chrono::ticks::zero()) {
            throw std::invalid_argument("Rain duration must be positive");
        }
        mNextState = {Type::Rain, time};
    }
    void setThunder(ll::chrono::ticks time) {
        if (time <= ll::chrono::ticks::zero()) {
            throw std::invalid_argument("Thunder duration must be positive");
        }
        mNextState = {Type::Thunder, time};
    }
};

class WeatherUpdatedEvent final : public WeatherUpdateEvent {
public:
    using WeatherUpdateEvent::WeatherUpdateEvent;
};

} // namespace ila::mc::inline world