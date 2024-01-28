#pragma once

#include "src/time_util.hpp"

namespace dynamic_paper_test {

constexpr time_t ONE_SECOND = 1;
constexpr time_t ONE_MINUTE = ONE_SECOND * 60;
constexpr time_t ONE_HOUR = ONE_MINUTE * 60;

const dynamic_paper::SunriseAndSunsetTimes
    testSunriseAndSunsetTimes(dynamic_paper::DUMMY_SUNRISE_TIME,
                              dynamic_paper::DUMMY_SUNSET_TIME);

const time_t ZERO_TIME =
    dynamic_paper::timeStringToTime("00:00", testSunriseAndSunsetTimes).value();

const time_t SUNRISE_SUNSET_GAP =
    testSunriseAndSunsetTimes.sunset - testSunriseAndSunsetTimes.sunrise;

} // namespace dynamic_paper_test
