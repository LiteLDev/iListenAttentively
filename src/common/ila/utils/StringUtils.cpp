#include "ila/utils/StringUtils.i.h"
#include <cctype>
#include <ll/api/base/StdInt.h>
#include <ranges>
#include <string>

namespace ila::inline utils::string_utils {

std::string camelToSnakeWithoutM(std::string_view input) {
    if (input.empty()) return std::string{input};

    std::string_view sv = input;
    if (sv.front() == 'm') sv.remove_prefix(1);

    std::string result;
    result.reserve(sv.size() * 2);

    for (size_t i = 0; i < sv.size(); ++i) {
        int8 c = sv[i];
        if (std::isupper(static_cast<uint8>(c))) {
            if (i != 0) {
                result += '_';
            }
            result += static_cast<int8>(std::tolower(static_cast<uint8>(c)));
        } else {
            result += c;
        }
    }
    return result;
}

std::string snakeToCamelWithM(std::string_view input) {
    if (input.empty()) return "m";

    std::string result;
    result.reserve(input.size() * 2);
    result += 'm';

    // clang-format off
    result += input
        | std::views::split('_')
        | std::views::transform([](auto&& word) -> std::string {
            if (word.empty()) return {};
            std::string result{word.begin(), word.end()};
            result[0] = static_cast<int8>(std::toupper(static_cast<uint8>(result[0])));
            return result;
        })
        | std::views::join
        | std::ranges::to<std::string>();
    // clang-format on

    return result;
}

} // namespace ila::inline utils::string_utils