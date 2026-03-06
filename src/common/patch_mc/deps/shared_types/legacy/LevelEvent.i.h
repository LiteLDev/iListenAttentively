#pragma once
#include <magic_enum.hpp>
#include <mc/deps/shared_types/legacy/LevelEvent.h>

template <>
struct magic_enum::customize::enum_range<SharedTypes::Legacy::LevelEvent> {
    static constexpr auto min = static_cast<int>(SharedTypes::Legacy::LevelEvent::Undefined);
    static constexpr auto max = static_cast<int>(SharedTypes::Legacy::LevelEvent::ParticleLegacyEvent);
};