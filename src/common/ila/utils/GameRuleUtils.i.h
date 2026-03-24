#pragma once
#include <mc/world/level/storage/GameRules.h>

namespace ila::inline utils::gamerule_utils {

template <typename T>
    requires(ll::traits::is_in_types_v<T, GameRule::Value>)
optional_ref<T> getGameRule(GameRules& gameRules, GameRules::GameRulesIndex id) {
    auto index = static_cast<size_t>(std::to_underlying(id));
    if (gameRules.mGameRules->size() <= index) return std::nullopt;
    auto& gameRule = (*gameRules.mGameRules)[index];
    if (auto* result = std::get_if<T>(&*gameRule.mValue); result) return *result;
    return std::nullopt;
}

template <typename T>
    requires(ll::traits::is_in_types_v<T, GameRule::Value>)
T getGameRule(GameRules& gameRules, GameRules::GameRulesIndex id, T defaultValue) {
    return getGameRule<T>(gameRules, id).value_or(defaultValue);
}

} // namespace ila::inline utils::memory_utils