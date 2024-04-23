#pragma once

/**
 * Represent the amount of time that has passed since midnight of the current
 * day
 */

#include <chrono>
#include <ostream>
#include <type_traits>

#include "math_util.hpp"

namespace dynamic_paper {

/** Restriction to make it only possible to add time units <= 1 day to
 * `TimeFromMidnight` */
template <typename T>
concept DayOrShorter = std::is_same_v<T, std::chrono::seconds> ||
                       std::is_same_v<T, std::chrono::minutes> ||
                       std::is_same_v<T, std::chrono::hours> ||
                       std::is_same_v<T, std::chrono::day>;

/**
 * Class to represent the amount of time from midnight of the current day
 * Will never have a value greater than a single day (seconds is always in the
 * range [0..<(24h in seconds)]).
 * Adding/Subtracting durations to this will loop
 * back to the start/end of the day if the value would be before 00:00:00 or
 * after 23:59:59.
 */
class TimeFromMidnight {
public:
  constexpr operator std::chrono::seconds() const { return seconds; }

  inline TimeFromMidnight &operator=(const TimeFromMidnight &other) = default;

  constexpr auto operator<=>(const TimeFromMidnight &) const noexcept = default;
  template <DayOrShorter T>
  friend TimeFromMidnight operator<=>(const TimeFromMidnight &time, T rhs);

  template <DayOrShorter T>
  friend TimeFromMidnight operator+(const TimeFromMidnight &time, T rhs);
  friend TimeFromMidnight operator+(const TimeFromMidnight &lhs,
                                    const TimeFromMidnight &rhs);
  template <DayOrShorter T>
  friend TimeFromMidnight operator-(const TimeFromMidnight &time, T rhs);
  friend TimeFromMidnight operator-(const TimeFromMidnight &lhs,
                                    const TimeFromMidnight &rhs);

  friend std::ostream &operator<<(std::ostream &osStream,
                                  const TimeFromMidnight &value);

  constexpr TimeFromMidnight(const std::chrono::seconds seconds)
      : seconds(mod(seconds, TimeFromMidnight::DAY_LENGTH)) {}
  template <DayOrShorter T>
  constexpr TimeFromMidnight(T time) : seconds(std::chrono::seconds(time)) {}
  TimeFromMidnight(const TimeFromMidnight &) = default;
  TimeFromMidnight(TimeFromMidnight &&) = default;
  ~TimeFromMidnight() = default;

private:
  std::chrono::seconds seconds;
  static constexpr std::chrono::seconds DAY_LENGTH = std::chrono::hours(24);
};

// operator<=>
template <DayOrShorter T>
auto operator<=>(const TimeFromMidnight &time, const T rhs) {
  return time.seconds <=> rhs;
}

// operator+
template <DayOrShorter T>
inline TimeFromMidnight operator+(const TimeFromMidnight &time, const T rhs) {
  return {mod(time.seconds + rhs, TimeFromMidnight::DAY_LENGTH)};
}

inline TimeFromMidnight operator+(const TimeFromMidnight &lhs,
                                  const TimeFromMidnight &rhs) {
  return {mod(lhs.seconds + rhs.seconds, TimeFromMidnight::DAY_LENGTH)};
}

// operator-
template <DayOrShorter T>
TimeFromMidnight operator-(const TimeFromMidnight &time, const T rhs) {
  return {mod(time.seconds - rhs, TimeFromMidnight::DAY_LENGTH)};
}

inline TimeFromMidnight operator-(const TimeFromMidnight &lhs,
                                  const TimeFromMidnight &rhs) {
  return {mod(lhs.seconds - rhs.seconds, TimeFromMidnight::DAY_LENGTH)};
}

} // namespace dynamic_paper

// formatter
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
