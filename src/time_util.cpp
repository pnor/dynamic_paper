#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <format>
#include <string>

#include <sunset.h>
#include <tl/expected.hpp>

#include "config.hpp"
#include "location.hpp"
#include "logger.hpp"
#include "string_util.hpp"
#include "time_from_midnight.hpp"
#include "time_util.hpp"

namespace dynamic_paper {

// ===== static helper ====================

namespace {

// === string to time objects helper functions ===

void explainError(const LocationError &error) {
  switch (error) {
  case LocationError::RequestFailed: {
    logError("Unable to get location for user because the http request "
             "failed, so using fallback from config");
    break;
  }
  case dynamic_paper::LocationError::UnableParseJsonResponse: {
    logError("Unable to parse json response from http request for "
             "location, so using fallback from config");
    break;
  }
  case dynamic_paper::LocationError::UnableParseLatitudeOrLongitude: {
    logError("Unable to parse latitude or longitude http request for "
             "location, so using fallback from config");
    break;
  }
  }
}

std::pair<double, double> getLatitudeAndLongitude(const Config &config) {
  if (config.locationInfo.useLocationInfoOverSearch) {
    return config.locationInfo.latitudeAndLongitude;
  }

  return getUserLatitudeAndLongitude(config)
      .map_error(explainError)
      .value_or(config.locationInfo.latitudeAndLongitude);
}

int timeZoneOffset() {
  time_t timeNow = time(nullptr);
  struct tm timeStruct = {};

  localtime_r(&timeNow, &timeStruct);

  constexpr long HOUR = 60L * 60L;
  return static_cast<int>(timeStruct.tm_gmtoff / HOUR);
}

TimeFromMidnight minutesFromMidnightToTimFromMidnight(const double minutes) {
  constexpr int MINUTES_TO_SECONDS = 60;
  return {std::chrono::seconds(static_cast<int>(minutes * MINUTES_TO_SECONDS))};
}

SunriseAndSunsetTimes
getSunriseAndSunsetTimesUsingLocation(const Config &config) {
  // TODO
  // if override time, use that
  // if override location use that
  // else get location
  // then call sunset api

  const std::pair<double, double> latitudeAndLongitude =
      getLatitudeAndLongitude(config);

  SunSet sunset;
  sunset.setPosition(latitudeAndLongitude.first, latitudeAndLongitude.second,
                     timeZoneOffset());
  const double sunriseRawValue = sunset.calcSunrise();
  const double sunsetRawValue = sunset.calcSunset();

  return {.sunrise = minutesFromMidnightToTimFromMidnight(sunriseRawValue),
          .sunset = minutesFromMidnightToTimFromMidnight(sunsetRawValue)};
}

// TODO delete !!
// tl::expected<SunriseAndSunsetTimes, SunriseAndSetErrors>
// getSunriseAndSetUsingSunwait() {
//   const tl::expected<std::string, CommandExecError> sunwaitExpectation =
//       runCommandStdout("sunwait list");
//
//   if (!sunwaitExpectation.has_value()) {
//     logWarning(
//         "Unable to execute command in attempt to get sunrise/sunset times");
//     return tl::unexpected(SunriseAndSetErrors::UnableExecuteCommand);
//   }
//
//   const std::string &sunwaitResult = sunwaitExpectation.value();
//
//   std::smatch groupMatches{};
//   const std::regex sunwaitRegex(R"(^(\d\d:\d\d), (\d\d:\d\d)\n)");
//
//   const bool outputMatchResult =
//       tryRegexes(sunwaitResult, groupMatches, sunwaitRegex);
//
//   if (!outputMatchResult) {
//     logWarning(
//         "return output not expected when getting sunrise/sunset times: {}",
//         sunwaitResult);
//     return tl::unexpected(SunriseAndSetErrors::BadOutput);
//   }
//
//   // --
//   const std::optional<TimeFromMidnight> sunriseTime =
//       convertRawTimeStringToTimeOffset(std::string(groupMatches[1]));
//   const std::optional<TimeFromMidnight> sunsetTime =
//       convertRawTimeStringToTimeOffset(std::string(groupMatches[2]));
//
//   if (!sunriseTime.has_value()) {
//     logError("Unable to convert sunrise raw string to a time! string was {}",
//              std::string(groupMatches[1]));
//     return tl::unexpected(SunriseAndSetErrors::BadOutput);
//   }
//   if (!sunsetTime.has_value()) {
//     logError("Unable to convert sunset raw string to a time! string was {}",
//              std::string(groupMatches[2]));
//     return tl::unexpected(SunriseAndSetErrors::BadOutput);
//   }
//
//   return {{.sunrise = sunriseTime.value(), .sunset = sunsetTime.value()}};
// }

std::optional<TimeFromMidnight> sunsetOrRiseStringToTimeOffset(
    const SunriseAndSunsetTimes &sunriseAndSunsetTimes,
    const std::smatch &groupMatches) {
  std::string sunsetOrRiseStr = trim_copy(groupMatches[1].str());
  std::ranges::transform(
      sunsetOrRiseStr.begin(), sunsetOrRiseStr.end(), sunsetOrRiseStr.begin(),
      [](const char letter) { return std::tolower(letter); });

  if (sunsetOrRiseStr == "sunrise") {
    return sunriseAndSunsetTimes.sunrise;
  }

  if (sunsetOrRiseStr == "sunset") {
    return sunriseAndSunsetTimes.sunset;
  }

  return std::nullopt;
}

std::optional<TimeFromMidnight>
sunOffsetStringToTimeOffset(const SunriseAndSunsetTimes &sunriseAndSunsetTimes,
                            const std::smatch &groupMatches) {
  const std::string &addOrSubtractStr = groupMatches[1].str();
  const std::string &offsetStr = groupMatches[2].str();
  std::string sunsetOrRiseStr = trim_copy(groupMatches[3].str());
  std::ranges::transform(
      sunsetOrRiseStr.begin(), sunsetOrRiseStr.end(), sunsetOrRiseStr.begin(),
      [](const char letter) { return std::tolower(letter); });

  std::optional<TimeFromMidnight> timeOffset =
      convertRawTimeStringToTimeOffset(offsetStr);

  if (!timeOffset.has_value()) {
    logWarning("Unable to parse {} for a time offset", offsetStr);
    return std::nullopt;
  }
  if (!(sunsetOrRiseStr == "sunrise" || sunsetOrRiseStr == "sunset")) {
    logWarning("Time offset did not specify whether to use sunrise or sunset");
    return std::nullopt;
  }
  if (!(addOrSubtractStr == "+" || addOrSubtractStr == "-")) {
    logWarning("Unable to identify whether to add or subtract time offset from "
               "{} time",
               sunsetOrRiseStr);
    return std::nullopt;
  }

  const TimeFromMidnight baseSunTime = sunsetOrRiseStr == "sunrise"
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

} // namespace

// ===== header ====================

TimeFromMidnight dummySunriseTime() {
  return TimeFromMidnight::forTime("08:00");
}

TimeFromMidnight dummySunsetTime() {
  return TimeFromMidnight::forTime("20:00");
}

TimeFromMidnight getCurrentTime() {
  // HH:MM:SS
  constexpr size_t HOURS_MINUTES_SIZE = 8;
  constexpr size_t START_OF_LOCAL_TIME = 11;

  const std::chrono::zoned_time zonedTime{std::chrono::current_zone(),
                                          std::chrono::system_clock::now()};

  const std::string timeString =
      std::format("{}", zonedTime).substr(START_OF_LOCAL_TIME);

  logDebug("Current time unparsed is {}", timeString);

  std::optional<TimeFromMidnight> optTime = convertRawTimeStringToTimeOffset(
      timeString.substr(0, HOURS_MINUTES_SIZE));

  logAssert(optTime.has_value(), "Unable to parse valid time from return "
                                 "result of current time as string");

  return optTime.value();
}

SunriseAndSunsetTimes getSunriseAndSunsetTimes(const Config &config) {
  switch (config.sunEventPollerMethod) {
  case dynamic_paper::SunEventPollerMethod::Dummy: {
    return {.sunrise = dummySunriseTime(), .sunset = dummySunsetTime()};
  }
  default: {
    return getSunriseAndSunsetTimesUsingLocation(config);
  }
  }
}

std::optional<TimeFromMidnight>
timeStringToTime(const std::string &origString,
                 const SunriseAndSunsetTimes &sunriseAndSunsetTimes) {

  std::string timeString = trim_copy(origString);

  // "sunrise" or "sunset"
  const std::regex sunRegex("\\s*(sunrise|sunset)\\s*",
                            std::regex_constants::icase);

  // (+/-) xx:xx (sunrise/sunset)
  const std::regex sunOffsetRegex(R"(^(\+|-)\s*(\d+:\d\d)\s*(sunrise|sunset))",
                                  std::regex_constants::icase);

  // (+/-) xx:xx:xx (sunrise/sunset)
  const std::regex sunOffsetSecondsRegex(
      R"(^(\+|-)\s*(\d+:\d\d:\d\d)\s*(sunrise|sunset))",
      std::regex_constants::icase);

  // xx:xx
  const std::regex timeRegex(R"(^(\d+:\d\d))", std::regex_constants::icase);

  // xx:xx:xx
  const std::regex timeWithSecondsRegex(R"(^(\d+:\d\d:\d\d))",
                                        std::regex_constants::icase);

  std::smatch groupMatches;

  if (tryRegexes(timeString, groupMatches, sunRegex)) {
    return sunsetOrRiseStringToTimeOffset(sunriseAndSunsetTimes, groupMatches);
  }

  if (tryRegexes(timeString, groupMatches, sunOffsetRegex,
                 sunOffsetSecondsRegex)) {
    return sunOffsetStringToTimeOffset(sunriseAndSunsetTimes, groupMatches);
  }

  if (tryRegexes(timeString, groupMatches, timeRegex, timeWithSecondsRegex)) {
    const std::string &offsetStr = groupMatches[0].str();
    return convertRawTimeStringToTimeOffset(offsetStr);
  }

  logWarning("Unable to parse/match the time string: {}", timeString);
  return std::nullopt;
}

std::optional<std::vector<TimeFromMidnight>>
timeStringsToTimes(const std::vector<std::string> &strings,
                   const SunriseAndSunsetTimes &sunriseAndSunsetTimes) {
  std::vector<TimeFromMidnight> times;
  times.reserve(strings.size());

  for (const auto &timeString : strings) {
    std::optional<TimeFromMidnight> optTime =
        timeStringToTime(timeString, sunriseAndSunsetTimes);
    if (!optTime.has_value()) {
      return std::nullopt;
    }
    times.push_back(optTime.value());
  }

  return times;
}

} // namespace dynamic_paper
