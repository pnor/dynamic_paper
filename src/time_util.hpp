#pragma once

/** Handling of time based events */

#include <chrono>
#include <concepts>
#include <ctime>
#include <functional>

#include "config.hpp"
#include "time_from_midnight.hpp"

namespace dynamic_paper {

/** Time used for sunrise when testing using `SunEventPollerMethod::Dummy` */
TimeFromMidnight dummySunriseTime();

/** Time used for sunset when testing using `SunEventPollerMethod::Dummy` */
TimeFromMidnight dummySunsetTime();

/** Potential errors that can arrise when trying to poll for sunset and sunrise
 */
enum class SunriseAndSetErrors {
  CommandNotFound,
  BadOutput,
  UnableExecuteCommand
};

/** Storage class for sunrise and sunset times */
struct SunriseAndSunsetTimes {
  TimeFromMidnight sunrise;
  TimeFromMidnight sunset;
};

/**
 * Returns the current time using the system clock, as seconds from midnight
 */
TimeFromMidnight getCurrentTime();

/**
 * Returns the time of sunrise and sunset by querying another program specified
 * by the Config.
 *
 * Returns nullopt if the query is unsucessful (cmd is not installed, cmd
 * failed, etc.)
 */
SunriseAndSunsetTimes getSunriseAndSunsetTimes(const Config &config);

/**
 *  Convert string formatted HH:MM or HH:MM:SS to number of seconds from
 * midnight
 *
 *  Example:
 *
 *  01:00 -> 3600
 *  02:00 -> 7200
 *  10:00 -> 36000
 *  10:00:01 -> 36001
 */
constexpr std::optional<TimeFromMidnight>
convertRawTimeStringToTimeOffset(std::string_view timeString);

/**
 *  Convert string formatted HH:MM or HH:MM:SS to number of seconds, without
 * checking to see if the checking the format of `timeString` represents a valid
 * time string. Will throw an error if it does not.
 */
constexpr TimeFromMidnight
convertRawTimeStringToTimeOffsetUnchecked(std::string_view timeString);

/**
 * Converts a string formatted either a raw time
 * "11:00"
 * Or offset from sunrise or sunset:
 * "-01:00 sunrise"
 * "+01:30 sunset"
 * and converts it to a `TimeFromMidnight`, the number of seconds from the start
 * of the day.
 *
 * Does this by converting the time to the time in the UTC timezone
 * on 1900-01-00 and the HH:MM based on the string.
 */
std::optional<TimeFromMidnight>
timeStringToTime(const std::string &origString,
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
std::optional<std::vector<TimeFromMidnight>>
timeStringsToTimes(const std::vector<std::string> &strings,
                   const SunriseAndSunsetTimes &sunriseAndSunsetTimes);

/**
 * Returns the amount of time it took to run `block`
 */
std::chrono::milliseconds elapsedTimeToRunCodeBlock(std::invocable auto block);

// ===== constexpr definition =======

constexpr bool isDigit(const char character) {
  return character == '0' || character == '1' || character == '2' ||
         character == '3' || character == '4' || character == '5' ||
         character == '6' || character == '7' || character == '8' ||
         character == '9';
}

/**
 * Returns a positive integer contained in `s` or `nullopt` if unable to parse.
 * `s` should contain strictly numbers, no (+/-) or decimals.
 */
constexpr std::optional<unsigned int>
stringViewToInt(const std::string_view text) {
  int res = 0;
  for (const char character : text) {
    res *= 10;
    if (isDigit(character)) {
      res += (character - '0');
    } else {
      return std::nullopt;
    }
  }
  return res;
}

constexpr std::optional<TimeFromMidnight>
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

  return hours + seconds + minutes;
}

constexpr TimeFromMidnight
convertRawTimeStringToTimeOffsetUnchecked(std::string_view timeString) {
  std::optional<TimeFromMidnight> optTime =
      convertRawTimeStringToTimeOffset(timeString);
  return optTime.value();
}

std::chrono::milliseconds timeToRunCodeBlock(std::invocable auto block) {
  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();

  std::invoke(block);

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
}

} // namespace dynamic_paper
