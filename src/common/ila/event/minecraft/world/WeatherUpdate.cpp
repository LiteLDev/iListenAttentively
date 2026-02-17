#include "ila/event/minecraft/world/WeatherUpdate.h"
#include "ila/base/Gloabl.hpp"

namespace ila::mc::inline world {

void WeatherUpdateEvent::serialize(CompoundTag& nbt) const {
    nbt["prev_type"]     = magic_enum::enum_name(mPrevState.first);
    nbt["prev_duration"] = mPrevState.second.count();
    nbt["next_type"]     = magic_enum::enum_name(mNextState.first);
    nbt["next_duration"] = mNextState.second.count();
}

void WeatherUpdatingEvent::deserialize(CompoundTag const& nbt) {
    mPrevState.first  = ll::reflection::deserialize_to<Type>(nbt["prev_type"]).value();
    mPrevState.second = ll::chrono::ticks{nbt["prev_duration"]};
    mNextState.first  = ll::reflection::deserialize_to<Type>(nbt["next_type"]).value();
    mNextState.second = ll::chrono::ticks{nbt["next_duration"]};
}

} // namespace ila::mc::inline world