#include "time_util.hpp"

#include <algorithm>
#include <assert.h>
#include <cctype>
#include <ctime>
#include <expected>
#include <locale>
#include <regex>

#include "command_executor.hpp"
#include "config.hpp"
#include "logger.hpp"

namespace dynamic_paper {

// ===== static helper ====================

// === string manip ===

// trim from start (in place)
static inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
  rtrim(s);
  ltrim(s);
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s) {
  trim(s);
  return s;
}

// === string to time objects helper functions ===

/**
 * Converts a string formatted HH:MM to `std::tm`
 */
static std::tm hourMinuteStringToTM(const std::string &hourMinutes) {
  std::istringstream time(hourMinutes);
  std::tm timeTm = {};
  time >> std::get_time(&timeTm, "%H:%M");
  return timeTm;
}

static std::expected<SunriseAndSunsetTimes, SunriseAndSetErrors>
getSunriseAndSetUsingSunwait() {
  const std::expected<std::string, CommandExecError> sunwaitExpectation =
      runCommandStdout("sunwait list");

  if (!sunwaitExpectation.has_value()) {
    logWarning(
        "Unable to execute command in attempt to get sunrise/sunset times");
    return std::unexpected(SunriseAndSetErrors::UnableExecuteCommand);
  }

  const std::string &sunwaitResult = sunwaitExpectation.value();

  if (sunwaitResult.size() < 12) {
    logWarning(
        "return output not expected when getting sunrise/sunset times: " +
        sunwaitResult);
    return std::unexpected(SunriseAndSetErrors::BadOutput);
  }

  std::tm sunriseTm = hourMinuteStringToTM(sunwaitResult.substr(0, 5));
  std::tm sunsetTm = hourMinuteStringToTM(sunwaitResult.substr(7, 5));

  return SunriseAndSunsetTimes(mktime(&sunriseTm), mktime(&sunsetTm));
}

template <typename... Ts>
static inline bool tryRegexes(const std::string &s, std::smatch &groupMatches,
                              const std::regex regex, Ts... otherRegexes) {
  if constexpr (sizeof...(Ts) == 0) {
    return std::regex_match(s, groupMatches, regex);
  } else {
    return std::regex_match(s, groupMatches, regex) ||
           tryRegexes(s, groupMatches, otherRegexes...);
  }
}
static std::optional<time_t> sunsetOrRiseStringToTimeOffset(
    const SunriseAndSunsetTimes &sunriseAndSunsetTimes,
    const std::smatch &groupMatches) {
  std::string sunsetOrRiseStr = trim_copy(groupMatches[1].str());
  std::ranges::transform(sunsetOrRiseStr.begin(), sunsetOrRiseStr.end(),
                         sunsetOrRiseStr.begin(),
                         [](const char c) { return std::tolower(c); });
  if (sunsetOrRiseStr == "sunrise") {
    return sunriseAndSunsetTimes.sunrise;
  } else if (sunsetOrRiseStr == "sunset") {
    return sunriseAndSunsetTimes.sunset;
  } else {
    return std::nullopt;
  }
}

static std::optional<time_t>
sunOffsetStringToTimeOffset(const SunriseAndSunsetTimes &sunriseAndSunsetTimes,
                            const std::smatch &groupMatches) {
  const std::string &addOrSubtractStr = groupMatches[1].str();
  const std::string &offsetStr = groupMatches[2].str();
  std::string sunsetOrRiseStr = trim_copy(groupMatches[3].str());
  std::ranges::transform(sunsetOrRiseStr.begin(), sunsetOrRiseStr.end(),
                         sunsetOrRiseStr.begin(),
                         [](const char c) { return std::tolower(c); });

  std::optional<time_t> timeOffset =
      convertRawTimeStringToTimeOffset(offsetStr);

  if (!timeOffset.has_value()) {
    logWarning("Unable to parse " + offsetStr + " for a time offset");
    return std::nullopt;
  }
  if (!(sunsetOrRiseStr == "sunrise" || sunsetOrRiseStr == "sunset")) {
    logWarning("Time offset did not specify whether to use sunrise or sunset");
    return std::nullopt;
  }
  if (!(addOrSubtractStr == "+" || addOrSubtractStr == "-")) {
    logWarning(
        "Unable to identify whether to add or subtract time offset from " +
        sunsetOrRiseStr + " time");
    return std::nullopt;
  }

  time_t baseSunTime = sunsetOrRiseStr == "sunrise"
                           ? sunriseAndSunsetTimes.sunrise
                           : sunriseAndSunsetTimes.sunset;

  switch (addOrSubtractStr[0]) {
  case '+': {
    return baseSunTime + timeOffset.value();
  };
  case '-': {
    return baseSunTime - timeOffset.value();
  }
  default: {
    logError("Reached impossible case in time parsing logic");
    return std::nullopt;
  }
  }
}

// ===== header ====================

std::optional<time_t>
convertRawTimeStringToTimeOffset(const std::string_view timeString) {
  size_t colonPos = timeString.find(':');
  if (colonPos == std::string::npos) {
    return std::nullopt;
  }

  std::string hoursSubstr(timeString.substr(0, colonPos));
  std::string minutesSubstr(timeString.substr(colonPos + 1));

  // make sure minutes is in format MM
  if (!(minutesSubstr.size() == 2)) {
    return std::nullopt;
  }

  int hours = std::stoi(hoursSubstr);
  int minutes = std::stoi(minutesSubstr);

  if ((hours < 0) || (minutes < 0 || minutes > 59)) {
    return std::nullopt;
  }

  return 60 * (minutes + (60 * hours));
}

std::optional<SunriseAndSunsetTimes>
getSunriseAndSetString(const Config &config) {
  switch (config.sunEventPollerMethod) {
  case SunEventPollerMethod::Sunwait: {
    std::expected<SunriseAndSunsetTimes, SunriseAndSetErrors> expectation =
        getSunriseAndSetUsingSunwait();

    if (expectation.has_value()) {
      return std::make_optional(expectation.value());
    } else {
      switch (expectation.error()) {
      case SunriseAndSetErrors::BadOutput: {
        logWarning("Unable to determine time of sunset or sunrise due to bad "
                   "output from the sunpolling program");
        break;
      }
      case SunriseAndSetErrors::CommandNotFound: {
        logWarning("Unable to determine time of sunset or sunrise due to bad "
                   "output from the sunpolling program");
        break;
      }
      case SunriseAndSetErrors::UnableExecuteCommand: {
        logWarning("Unable to determine time of sunset or sunrise due to not "
                   "being able to execute the sunpolling program");
        break;
      }
      }
      return std::nullopt;
    }
  };
  case SunEventPollerMethod::Dummy: {
    return SunriseAndSunsetTimes(DUMMY_SUNRISE_TIME, DUMMY_SUNSET_TIME);
  };
  default: {
    return std::nullopt;
  };
  }
}

std::optional<time_t>
timeStringToTime(const std::string &s,
                 const SunriseAndSunsetTimes &sunriseAndSunsetTimes) {

  std::string timeString = trim_copy(s);

  const std::regex sunRegex("\\s*(sunrise|sunset)\\s*",
                            std::regex_constants::icase);
  const std::regex sunOffsetRegex("^(\\+|-)(\\d\\d:\\d\\d)\\s(sunrise|sunset)",
                                  std::regex_constants::icase);
  const std::regex sunOffsetHourDigitsRegex(
      "^(\\+|-)(\\d+:\\d\\d)\\s(sunrise|sunset)", std::regex_constants::icase);
  const std::regex timeRegex("^(\\d+\\d:\\d\\d)", std::regex_constants::icase);
  const std::regex timeRegexHoursDigitsRegex("^(\\d+:\\d\\d)",
                                             std::regex_constants::icase);
  std::smatch groupMatches;

  if (tryRegexes(s, groupMatches, sunRegex)) {
    return sunsetOrRiseStringToTimeOffset(sunriseAndSunsetTimes, groupMatches);
  } else if (tryRegexes(s, groupMatches, sunOffsetRegex,
                        sunOffsetHourDigitsRegex)) {
    return sunOffsetStringToTimeOffset(sunriseAndSunsetTimes, groupMatches);
  } else if (tryRegexes(s, groupMatches, timeRegex,
                        timeRegexHoursDigitsRegex)) {
    const std::string &offsetStr = groupMatches[0].str();
    return convertRawTimeStringToTimeOffset(offsetStr);
  } else {
    logWarning("Unable to parse/match the time string: " + timeString);
    return std::nullopt;
  }
}

std::optional<std::vector<time_t>>
timeStringsToTimes(const std::vector<std::string> &strings,
                   const SunriseAndSunsetTimes &sunriseAndSunsetTimes) {
  std::vector<time_t> times;
  times.reserve(strings.size());

  for (const auto &s : strings) {
    std::optional<time_t> optTime = timeStringToTime(s, sunriseAndSunsetTimes);
    if (!optTime.has_value()) {
      return std::nullopt;
    } else {
      times.push_back(optTime.value());
    }
  }

  return times;
}

} // namespace dynamic_paper
