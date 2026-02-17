#include "ila/utils/RandomColorLogFormatter.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fmt/format.h>
#include <ll/api/io/LogLevel.h>
#include <ll/api/io/LogMessage.h>
#include <ll/api/io/PatternFormatter.h>
#include <ll/api/utils/StringUtils.h>
#include <ll/api/utils/SystemUtils.h>
#include <mc/deps/core/math/Color.h>
#include <random>
#include <string>
#include <string_view>

namespace ila {

void RandomColorLogFormatter::format(ll::io::LogMessageView const& view, std::string& buffer) const noexcept {
    if (view.lvl != ll::io::LogLevel::Info) return PatternFormatter::format(view, buffer);

    mce::Color start, end;
    do {
        start = generateRandomBrightColor();
        end   = generateRandomBrightColor();
    } while (start.distanceTo(end) < 35.0);

    ll::string_utils::splitByPattern([&](std::string_view line) -> bool {
        PatternFormatter::format(
            ll::io::LogMessageView{
                gradientText(std::string{line}, start, end),
                gradientText(std::string{view.tit}, end, start),
                view.lvl,
                view.tm
            },
            buffer
        );
        return true;
    }, view.msg, "\n", true);
}

mce::Color RandomColorLogFormatter::hslToRgb(double h, double s, double l) const noexcept {
    h = fmod(fmod(h, 360.0) + 360.0, 360.0);

    auto c = (1.0 - fabs(2.0 * l - 1.0)) * s;
    auto x = c * (1.0 - fabs(fmod(h / 60.0, 2.0) - 1.0));
    auto m = l - c / 2.0;

    struct {
        double r, g, b;
    } cases[6] = {
        {c, x, 0}, // 0-59度
        {x, c, 0}, // 60-119度
        {0, c, x}, // 120-179度
        {0, x, c}, // 180-239度
        {x, 0, c}, // 240-299度
        {c, 0, x}  // 300-359度
    };

    auto h_prime = static_cast<int>(h / 60.0) % 6;
    h_prime      = h_prime < 0 ? h_prime + 6 : h_prime;

    return {
        std::clamp(static_cast<int>((cases[h_prime].r + m) * 255.0), 0, 255),
        std::clamp(static_cast<int>((cases[h_prime].g + m) * 255.0), 0, 255),
        std::clamp(static_cast<int>((cases[h_prime].b + m) * 255.0), 0, 255)
    };
}
mce::Color RandomColorLogFormatter::generateRandomBrightColor() const noexcept {
    std::random_device rd;
    std::mt19937       gen(rd());
    return hslToRgb(
        std::uniform_real_distribution<double>(0.0, 360.0)(gen),
        std::uniform_real_distribution<double>(0.7, 1.0)(gen),
        std::uniform_real_distribution<double>(0.7, 0.85)(gen)
    );
}

std::string RandomColorLogFormatter::gradientText(
    std::string const& str,
    mce::Color const&  start,
    mce::Color const&  end
) const noexcept {
    if (str.empty() || !ll::sys_utils::isStdoutSupportAnsi()) return str;
    std::string result;
    for (size_t index = 0, length = str.size(); index < length; ++index) {
        auto interpolation_factor  = length == 1 ? 0.5 : static_cast<double>(index) / static_cast<double>(length - 1);
        result                    += fmt::format(
            "\033[38;2;{};{};{}m{}\033[0m",
            static_cast<uint8_t>(std::round((start.r + (end.r - start.r) * interpolation_factor) * 255.0)),
            static_cast<uint8_t>(std::round((start.g + (end.g - start.g) * interpolation_factor) * 255.0)),
            static_cast<uint8_t>(std::round((start.b + (end.b - start.b) * interpolation_factor) * 255.0)),
            str.substr(index, 1)
        );
    }
    return result;
}

} // namespace ila