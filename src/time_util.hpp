#pragma once

#include <chrono>
#include <ctime>

#include "config.hpp"

/** Handling of time based events */

namespace dynamic_paper {

/** Time used for sunrise when testing using `SunEventPollerMethod::Dummy` */
constexpr time_t DUMMY_SUNRISE_TIME = 8L * (60L * 60L);
/** Time used for sunset when testing using `SunEventPollerMethod::Dummy` */
constexpr time_t DUMMY_SUNSET_TIME = 20L * (60L * 60L);

/** Potential errors that can arrise when trying to poll for sunset and sunrise
 */
enum class SunriseAndSetErrors {
  CommandNotFound,
  BadOutput,
  UnableExecuteCommand
};

/** Storage class for sunrise and sunset times */
struct SunriseAndSunsetTimes {
  time_t sunrise;
  time_t sunset;

  SunriseAndSunsetTimes(const time_t sunrise, const time_t sunset)
      : sunrise(sunrise), sunset(sunset) {}
};

/**
 * Returns the time of sunrise and sunset by querying another program specified
 * by the Config.
 *
 * Returns nullopt if the query is unsucessful (cmd is not installed, cmd
 * failed, etc.)
 */
std::optional<SunriseAndSunsetTimes>
getSunriseAndSetString(const Config &config);

/**
 *  Convert string formatted HH:MM or HH:MM:SS to number of seconds
 *  Example:
 *  01:00 -> 3600
 *  02:00 -> 7200
 *  10:00 -> 36000
 *  10:00:01 -> 36001
 */
constexpr std::optional<time_t>
convertRawTimeStringToTimeOffset(const std::string_view timeString);

/**
 * Converts a string formatted either a raw time
 * "11:00"
 * Or offset from sunrise or sunset:
 * "-01:00 sunrise"
 * "+01:30 sunset"
 * and converts it to a `time_t` in the UTC timezone on 1900-01-00 and the HH:MM
 * based on the string.
 */
std::optional<time_t>
timeStringToTime(const std::string &strings,
                 const SunriseAndSunsetTimes &sunriseAndSunsetTimes);

/**
 * Converts a vector of strings formatted either a raw time
 * "11:00"
 * Or offset from sunrise or sunset:
 * "-01:00 sunrise"
 * "+01:30 sunset"
 * and converts it to a `time_t` in the UTC timezone on 1900-01-00 and the HH:MM
 * based on the string.
 *
 * If anyone 1 string is unable to be parsed, then the function returns nullopt
 */
std::optional<std::vector<time_t>>
timeStringsToTimes(const std::vector<std::string> &strings,
                   const SunriseAndSunsetTimes &sunriseAndSunsetTimes);

// ===== constexpr definition =====

/**
 * Returns a positive integer contained in `s` or `nullopt` if unable to parse.
 * `s` should contain strictly numbers, no (+/-) or decimals.
 */
constexpr std::optional<unsigned int>
stringViewToInt(const std::string_view s) {
  int res = 0;
  for (std::string_view::size_type i = 0; i < s.size(); i++) {
    res *= 10;
    if (std::isdigit(s[i])) {
      res += (s[i] - '0');
    } else {
      return std::nullopt;
    }
  }
  return res;
}

constexpr std::optional<time_t>
convertRawTimeStringToTimeOffset(const std::string_view timeString) {
  size_t colonPos = timeString.find(':');
  if (colonPos == std::string::npos) {
    return std::nullopt;
  }

  size_t secondColonPos = timeString.find(':', colonPos + 1);
  if (secondColonPos == std::string::npos) {
    secondColonPos = timeString.size();
  }

  const std::string_view hoursSubstr(timeString.substr(0, colonPos));
  const std::string_view minutesSubstr(
      timeString.substr(colonPos + 1, secondColonPos - colonPos - 1));
  const std::string_view secondsSubstr =
      secondColonPos >= timeString.size() - 1
          ? ""
          : (timeString.substr(secondColonPos + 1));

  // make sure minutes is in format MM
  if (minutesSubstr.size() != 2) {
    return std::nullopt;
  }
  // make sure seconds is in format ss or is empty
  if (secondsSubstr.size() == 1 || secondsSubstr.size() > 2) {
    return std::nullopt;
  }

  std::optional<unsigned int> parsedHours = stringViewToInt(hoursSubstr);
  if (!parsedHours.has_value()) {
    return std::nullopt;
  }
  std::optional<unsigned int> parsedMinutes = stringViewToInt(minutesSubstr);
  if (!parsedMinutes.has_value()) {
    return std::nullopt;
  }
  std::optional<unsigned int> parsedSeconds = stringViewToInt(secondsSubstr);
  if (!parsedSeconds.has_value()) {
    return std::nullopt;
  }

  std::chrono::hours hours(parsedHours.value());
  std::chrono::minutes minutes(parsedMinutes.value());
  std::chrono::seconds seconds(parsedSeconds.value());

  if ((hours.count() < 0) || (minutes.count() < 0 || minutes.count() > 59) ||
      (seconds.count() < 0 || seconds.count() > 59)) {
    return std::nullopt;
  }

  return std::chrono::seconds(hours + seconds + minutes).count();
}
} // namespace dynamic_paper
