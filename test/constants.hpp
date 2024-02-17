#pragma once

#include "src/time_util.hpp"

namespace dynamic_paper_test {

// ===== Helper ==========

namespace _helper {

consteval time_t getZeroTime() {
  const std::optional<time_t> time =
      dynamic_paper::convertRawTimeStringToTimeOffset("00:00");
  dynamic_paper::logAssert(time.has_value(),
                           "Zero time did not have value in constexpr");
  return time.value();
}

} // namespace _helper

// ===== Header ==========

constexpr time_t ONE_SECOND = 1;
constexpr time_t ONE_MINUTE = ONE_SECOND * 60;
constexpr time_t ONE_HOUR = ONE_MINUTE * 60;
constexpr time_t TWENTY_FOUR_HOURS = ONE_HOUR * 24;

const dynamic_paper::SunriseAndSunsetTimes testSunriseAndSunsetTimes = {
    .sunrise = dynamic_paper::DUMMY_SUNRISE_TIME,
    .sunset = dynamic_paper::DUMMY_SUNSET_TIME};

constexpr time_t ZERO_TIME = _helper::getZeroTime();

const time_t SUNRISE_SUNSET_GAP =
    testSunriseAndSunsetTimes.sunset - testSunriseAndSunsetTimes.sunrise;

} // namespace dynamic_paper_test
