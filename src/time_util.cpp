#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <format>
#include <string>

#include <boost/xpressive/xpressive_static.hpp>
#include <sunset.h>
#include <tl/expected.hpp>

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
  case LocationError::UnableParseJsonResponse: {
    logError("Unable to parse json response from http request for "
             "location, so using fallback from config");
    break;
  }
  case LocationError::UnableParseLatitudeOrLongitude: {
    logError("Unable to parse latitude or longitude http request for "
             "location, so using fallback from config");
    break;
  }
  }
}

std::pair<double, double>
getLatitudeAndLongitude(const LocationInfo &locationInfo) {
  if (locationInfo.useLatitudeAndLongitudeOverLocationSearch) {
    return locationInfo.latitudeAndLongitude;
  }

  return getLatitudeAndLongitudeFromHttp()
      .map_error(explainError)
      .value_or(locationInfo.latitudeAndLongitude);
}

int timeZoneOffset() {
  const time_t timeNow = time(nullptr);
  struct tm timeStruct = {};

  localtime_r(&timeNow, &timeStruct);

  constexpr long HOUR = 60L * 60L;
  return static_cast<int>(timeStruct.tm_gmtoff / HOUR);
}

TimeFromMidnight minutesFromMidnightToTimFromMidnight(const double minutes) {
  constexpr int MINUTES_TO_SECONDS = 60;
  return {std::chrono::seconds(static_cast<int>(minutes * MINUTES_TO_SECONDS))};
}

std::optional<TimeFromMidnight>
sunsetOrRiseStringToTimeOffset(const SolarDay &solarDay,
                               const boost::xpressive::smatch &groupMatches) {
  std::string sunsetOrRiseStr = trim_copy(groupMatches[1].str());
  std::ranges::transform(
      sunsetOrRiseStr.begin(), sunsetOrRiseStr.end(), sunsetOrRiseStr.begin(),
      [](const char letter) { return std::tolower(letter); });

  if (sunsetOrRiseStr == "sunrise") {
    return solarDay.sunrise;
  }

  if (sunsetOrRiseStr == "sunset") {
    return solarDay.sunset;
  }

  return std::nullopt;
}

std::optional<TimeFromMidnight>
sunOffsetStringToTimeOffset(const SolarDay &solarDay,
                            const boost::xpressive::smatch &groupMatches) {
  const std::string &addOrSubtractStr = groupMatches[1].str();
  const std::string &offsetStr = groupMatches[2].str();
  std::string sunsetOrRiseStr = trim_copy(groupMatches[3].str());
  std::ranges::transform(
      sunsetOrRiseStr.begin(), sunsetOrRiseStr.end(), sunsetOrRiseStr.begin(),
      [](const char letter) { return std::tolower(letter); });

  std::optional<TimeFromMidnight> timeOffset =
      convertTimeStringToTimeFromMidnight(offsetStr);

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

  const TimeFromMidnight baseSunTime =
      sunsetOrRiseStr == "sunrise" ? solarDay.sunrise : solarDay.sunset;

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

TimeFromMidnight getCurrentTime() {
  // HH:MM:SS
  constexpr size_t HOURS_MINUTES_SIZE = 8;
  constexpr size_t START_OF_LOCAL_TIME = 11;

  const std::chrono::zoned_time zonedTime{std::chrono::current_zone(),
                                          std::chrono::system_clock::now()};

  const std::string timeString =
      std::format("{}", zonedTime).substr(START_OF_LOCAL_TIME);

  logDebug("Current time unparsed is {}", timeString);

  std::optional<TimeFromMidnight> optTime = convertTimeStringToTimeFromMidnight(
      timeString.substr(0, HOURS_MINUTES_SIZE));

  logAssert(optTime.has_value(), "Unable to parse valid time from return "
                                 "result of current time as string");

  return optTime.value();
}

SolarDay getSolarDayUsingLocation(const LocationInfo &locationInfo) {
  const std::pair<double, double> latitudeAndLongitude =
      getLatitudeAndLongitude(locationInfo);

  SunSet sunset;
  sunset.setPosition(latitudeAndLongitude.first, latitudeAndLongitude.second,
                     timeZoneOffset());
  const double sunriseRawValue = sunset.calcSunrise();
  const double sunsetRawValue = sunset.calcSunset();

  return {.sunrise = minutesFromMidnightToTimFromMidnight(sunriseRawValue),
          .sunset = minutesFromMidnightToTimFromMidnight(sunsetRawValue)};
}

std::optional<TimeFromMidnight> timeStringToTime(const std::string &origString,
                                                 const SolarDay &solarDay) {

  const std::string timeString = trim_copy(origString);

  using namespace boost::xpressive;

  const boost::xpressive::sregex solarDaySubregex =
      (icase("sunrise") | icase("sunset"));

  // "sunrise" or "sunset"
  const boost::xpressive::sregex sunRegex =
      *_s >> (s1 = solarDaySubregex) >> *_s;

  // (+/-) xx:xx (sunrise/sunset)
  const boost::xpressive::sregex sunOffsetRegex =
      *_s >> (s1 = as_xpr('+') | '-') >> *_s >> (s2 = +_d >> ':' >> _d >> _d) >>
      *_s >> (s3 = solarDaySubregex) >> *_s;

  // (+/-) xx:xx:xx (sunrise/sunset)
  const boost::xpressive::sregex sunOffsetSecondsRegex =
      *_s >> (s1 = as_xpr('+') | '-') >> *_s >>
      (s2 = +_d >> ':' >> _d >> _d >> ':' >> _d >> _d) >> *_s >>
      (s3 = solarDaySubregex) >> *_s;

  // xx:xx
  const boost::xpressive::sregex timeRegex =
      *_s >> (s1 = +_d >> ':' >> _d >> _d) >> *_s;

  // xx:xx:xx
  const boost::xpressive::sregex timeWithSecondsRegex =
      *_s >> (s1 = +_d >> ':' >> _d >> _d >> ':' >> _d >> _d) >> *_s;

  boost::xpressive::smatch groupMatches;

  if (tryRegexes(timeString, groupMatches, sunRegex)) {
    return sunsetOrRiseStringToTimeOffset(solarDay, groupMatches);
  }

  if (tryRegexes(timeString, groupMatches, sunOffsetRegex,
                 sunOffsetSecondsRegex)) {
    return sunOffsetStringToTimeOffset(solarDay, groupMatches);
  }

  if (tryRegexes(timeString, groupMatches, timeRegex, timeWithSecondsRegex)) {
    const std::string &offsetStr = groupMatches[0].str();
    return convertTimeStringToTimeFromMidnight(offsetStr);
  }

  logWarning("Unable to parse/match the time string: {}", timeString);
  return std::nullopt;
}

std::optional<std::vector<TimeFromMidnight>>
timeStringsToTimes(const std::vector<std::string> &strings,
                   const SolarDay &solarDay) {
  std::vector<TimeFromMidnight> times;
  times.reserve(strings.size());

  for (const auto &timeString : strings) {
    std::optional<TimeFromMidnight> optTime =
        timeStringToTime(timeString, solarDay);
    if (!optTime.has_value()) {
      return std::nullopt;
    }
    times.push_back(optTime.value());
  }

  return times;
}

} // namespace dynamic_paper
