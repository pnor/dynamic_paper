#pragma once

#include <chrono>
#include <string>
#include <utility>

#ifdef use_fmt_lib
#include <fmt/core.h>
#else
#include <format>
#endif

#include "time_from_midnight.hpp"

/**
 * Formatting strings
 *
 * Since Apple Clang's std::format doesn't work in constexpr, this is
 * a layer to resolve that.
 * */

// TODO seems to not work with rvalues
// (temps returned from function calls)

namespace dynamic_paper {

#ifdef use_fmt_lib
template <typename... Ts> using FormatString = fmt::format_string<Ts...>;
#else
template <typename... Ts> using FormatString = std::format_string<Ts...>;
#endif

template <typename... Ts>
constexpr std::string format(const FormatString<Ts...> &msg, Ts &&...args) {
#ifdef use_fmt_lib
  return fmt::format<Ts...>(msg, std::forward<Ts>(args)...);
#else
  return std::format<Ts...>(msg, std::forward<Ts>(args)...);
#endif
}

} // namespace dynamic_paper

// ===== Formatter for custom types =================

#ifdef use_fmt_lib

// fmt::format
template <>
struct fmt::formatter<dydynamic_paper::TimeFromMidnight>
    : formatter<std::string> {
  auto format(const dydynamic_paper::TimeFromMidnight &time,
              fmt::format_context &ctx) const {
    return fmt::formatter<std::string>::format(
        fmt::format("{} from Midnight ({})", chrono::seconds(time),
                    std::format("{:%T}", chrono::seconds(time))),
        ctx);
  }
};

#else

// std::format
template <>
struct std::formatter<dynamic_paper::TimeFromMidnight>
    : std::formatter<std::string> {
  auto format(dynamic_paper::TimeFromMidnight time, format_context &ctx) const {
    return std::formatter<std::string>::format(
        std::format("{} from Midnight ({})", chrono::seconds(time),
                    std::format("{:%T}", chrono::seconds(time))),
        ctx);
  }
};

#endif
