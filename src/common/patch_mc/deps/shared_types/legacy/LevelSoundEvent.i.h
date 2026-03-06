#pragma once
#include <magic_enum.hpp>
#include <mc/deps/shared_types/legacy/LevelSoundEvent.h>

template <>
struct magic_enum::customize::enum_range<SharedTypes::Legacy::LevelSoundEvent> {
    static constexpr auto min = static_cast<int>(SharedTypes::Legacy::LevelSoundEvent::ItemUseOn);
    static constexpr auto max = static_cast<int>(SharedTypes::Legacy::LevelSoundEvent::Undefined);
};