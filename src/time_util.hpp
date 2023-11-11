#pragma once

#include <chrono>
#include <time.h>

#include "config.hpp"

/** Handling of time based events */

namespace dynamic_paper {

/** Time used for sunrise when testing using `SunEventPollerMethod::Dummy` */
constexpr time_t DUMMY_SUNRISE_TIME = 8 * (60 * 60);
/** Time used for sunset when testing using `SunEventPollerMethod::Dummy` */
constexpr time_t DUMMY_SUNSET_TIME = 20 * (60 * 60);

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

} // namespace dynamic_paper
