/**
 *   Test conversion of time strings to unix time
 *
 *   These tests care more about the relative differnce between conversions than
 * the resulting unix time.
 *
 *   Example:
 *   the amount of seconds between the conversion of "01:00" and "02:00" should
 * be:
 *
 * (60 * 60) = 3600
 *
 * the amount of seconds between the conversion of "-01:00 sunrise" and "+01:00
 * sunset" should be:
 *
 * (2 * (60 * 60)) + (<time between sunrise and sunset>)
 *
 */

#include <cassert>
#include <chrono>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include "helper.hpp"

#include "src/solar_day.hpp"
#include "src/time_from_midnight.hpp"
#include "src/time_util.hpp"

using namespace dynamic_paper;

// ===== Test Fixture ===============

class TimeStringConversion : public testing::Test {
public:
  void SetUp() override {}

  SolarDay testSolarDay = testSolarDayProvider().getSolarDay();

  std::chrono::seconds sunriseSunsetGap =
      testSolarDay.sunset - testSolarDay.sunrise;
};

// ===== Parsing HH:MM =====

TEST_F(TimeStringConversion, RawTimeTest) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"00:00", "01:00"}, testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], std::chrono::hours(1));
}

TEST_F(TimeStringConversion, RawTimeTest2) {
  std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"01:23", "04:55"}, testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0],
            std::chrono::hours(3) + std::chrono::minutes(32));
}

TEST_F(TimeStringConversion, RawTimeTest3) {
  std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"00:00", "23:59"}, testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0],
            std::chrono::hours(23) + std::chrono::minutes(59));
}

TEST_F(TimeStringConversion, RawTimeTest4) {
  std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"11:11", "22:55"}, testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0],
            std::chrono::hours(11) + std::chrono::minutes(44));
}

TEST_F(TimeStringConversion, RawTimeTest5) {
  std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"0:00", "1:00"}, testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], std::chrono::hours(1));
}

TEST_F(TimeStringConversion, RawTimeTestSeconds) {
  std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"00:00:00", "01:00:00", "01:00:01"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[0], std::chrono::seconds(0));
  EXPECT_EQ(times[2] - times[1], std::chrono::seconds(1));
}

TEST_F(TimeStringConversion, RawTimeTestSeconds2) {
  std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"01:00:12", "10:12:24"}, testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], std::chrono::hours(9) +
                                     std::chrono::minutes(12) +
                                     std::chrono::seconds(12));
}

TEST_F(TimeStringConversion, RawTimeTestSpaces) {
  std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes(
          {"   01:01:01    ", "    01:01:01", "01:01:01    "}, testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[0], std::chrono::hours(1) + std::chrono::minutes(1) +
                          std::chrono::seconds(1));
  EXPECT_EQ(times[1], std::chrono::hours(1) + std::chrono::minutes(1) +
                          std::chrono::seconds(1));
  EXPECT_EQ(times[2], std::chrono::hours(1) + std::chrono::minutes(1) +
                          std::chrono::seconds(1));
}

TEST_F(TimeStringConversion, BadRawTimeTestTooManyMinutes) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"01:90", "03:00"}, testSolarDay);

  EXPECT_FALSE(timesOpt.has_value());
}

TEST_F(TimeStringConversion, BadRawTimeTestTooLongString) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"001:90", "03:00"}, testSolarDay);

  EXPECT_FALSE(timesOpt.has_value());
}

TEST_F(TimeStringConversion, BadRawTimeTestTooLongString2) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"130:900", "03:00"}, testSolarDay);

  EXPECT_FALSE(timesOpt.has_value());
}

TEST_F(TimeStringConversion, BadRawTimeTestAtLeastOneIsBad) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes(
          {"00:00", "01:00", "2:00", "0003:00000", "4:00"}, testSolarDay);

  EXPECT_FALSE(timesOpt.has_value());
}

TEST_F(TimeStringConversion, BadRawTimeSecondsTooShort) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"00:00:0"}, testSolarDay);

  EXPECT_FALSE(timesOpt.has_value());
}

TEST_F(TimeStringConversion, BadRawTimeSecondsTooLong) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"00:00:000"}, testSolarDay);

  EXPECT_FALSE(timesOpt.has_value());
}

TEST_F(TimeStringConversion, BadRawTimeNoSeconds) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"00:00:"}, testSolarDay);

  EXPECT_FALSE(timesOpt.has_value());
}

// ===== Parsing +/-HH:MM sunrise/sunset =====

TEST_F(TimeStringConversion, SunoffsetTimeTestSunrise) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"+00:00 sunrise", "+01:00 sunrise"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], std::chrono::hours(1));
}

TEST_F(TimeStringConversion, SunoffsetTimeTestSunrise2) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"-00:01 sunrise", "-00:00 sunrise"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], std::chrono::minutes(1));
}

TEST_F(TimeStringConversion, SunoffsetTimeTestSunrise3) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"sunrise", "+01:30 sunrise"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0],
            std::chrono::hours(1) + std::chrono::minutes(30));
}

TEST_F(TimeStringConversion, SunoffsetTimeTestSunrise4) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"-2:20 sunrise", "+2:20 sunrise"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0],
            2 * (std::chrono::hours(2) + std::chrono::minutes(20)));
}

TEST_F(TimeStringConversion, SunoffsetTimeTestSunset) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"+00:00 sunset", "+01:00 sunset"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], std::chrono::hours(1));
}

TEST_F(TimeStringConversion, SunoffsetTimeTestSunset2) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"-00:01 sunset", "-00:00 sunset"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], std::chrono::minutes(1));
}

TEST_F(TimeStringConversion, SunoffsetTimeTestSunset3) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"sunset", "+01:30 sunset"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0],
            std::chrono::hours(1) + std::chrono::minutes(30));
}

TEST_F(TimeStringConversion, SunoffsetTimeTestSunset4) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"-2:20 sunset", "+2:20 sunset"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0],
            2 * (std::chrono::hours(2) + std::chrono::minutes(20)));
}

TEST_F(TimeStringConversion, SunoffsetTimeTestSunsetAndSunrise) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"sunrise", "sunset"}, testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], sunriseSunsetGap);
}

TEST_F(TimeStringConversion, SunoffsetTimeTestSunsetAndSunrise2) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"sunrise", "+3:00 sunset"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], sunriseSunsetGap + std::chrono::hours(3));
}

TEST_F(TimeStringConversion, SunoffsetTimeTestSunsetAndSunrise3) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"-10:11 sunrise", "+1:22 sunset"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0],
            sunriseSunsetGap +
                (std::chrono::hours(1) + std::chrono::minutes(22)) +
                (std::chrono::hours(10) + std::chrono::minutes(11)));
}

TEST_F(TimeStringConversion, SunriseCapitalization) {
  std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"SUNRISE", "+1:00 sunrise"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());
  std::vector<TimeFromMidnight> times = timesOpt.value();
  EXPECT_EQ(times[1] - times[0], std::chrono::hours(1));

  timesOpt = dynamic_paper::timeStringsToTimes({"sunrise", "+1:00 SUNRISE"},
                                               testSolarDay);
  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());
  times = timesOpt.value();
  EXPECT_EQ(times[1] - times[0], std::chrono::hours(1));

  timesOpt = dynamic_paper::timeStringsToTimes({"SUNRISE", "+1:00 SUNRISE"},
                                               testSolarDay);
  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());
  times = timesOpt.value();
  EXPECT_EQ(times[1] - times[0], std::chrono::hours(1));

  timesOpt = dynamic_paper::timeStringsToTimes({"sUnRiSe", "+1:00 sunRISE"},
                                               testSolarDay);
  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());
  times = timesOpt.value();
  EXPECT_EQ(times[1] - times[0], std::chrono::hours(1));
}

TEST_F(TimeStringConversion, SunsetCapitalization) {
  std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"SUNSET", "+1:00 sunset"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());
  std::vector<TimeFromMidnight> times = timesOpt.value();
  EXPECT_EQ(times[1] - times[0], std::chrono::hours(1));

  timesOpt = dynamic_paper::timeStringsToTimes({"sunset", "+1:00 SUNSET"},
                                               testSolarDay);
  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());
  times = timesOpt.value();
  EXPECT_EQ(times[1] - times[0], std::chrono::hours(1));

  timesOpt = dynamic_paper::timeStringsToTimes({"SUNSET", "+1:00 SUNSET"},
                                               testSolarDay);
  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());
  times = timesOpt.value();
  EXPECT_EQ(times[1] - times[0], std::chrono::hours(1));

  timesOpt = dynamic_paper::timeStringsToTimes({"sunset", "+1:00 sunset"},
                                               testSolarDay);
  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());
  times = timesOpt.value();
  EXPECT_EQ(times[1] - times[0], std::chrono::hours(1));
}

TEST_F(TimeStringConversion, SunriseEquivalence) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"sunrise", "sunrise"}, testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], std::chrono::seconds(0));
}

TEST_F(TimeStringConversion, SunriseEquivalence2) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"+1:00 sunrise", "+1:00 sunrise"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], std::chrono::seconds(0));
}

TEST_F(TimeStringConversion, SunriseEquivalence3) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"-1:00 sunrise", "-1:00 sunrise"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], std::chrono::seconds(0));
}

TEST_F(TimeStringConversion, SunsetEquivalence) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"sunset", "sunset"}, testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], std::chrono::seconds(0));
}

TEST_F(TimeStringConversion, SunsetEquivalence2) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"-1:00 sunset", "-01:00 sunset"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], std::chrono::seconds(0));
}

TEST_F(TimeStringConversion, SunsetEquivalence3) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"+1:00 sunset", "+01:00 sunset"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], std::chrono::seconds(0));
}

TEST_F(TimeStringConversion, SunoffsetTimeTestLongHour) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"+000:00 sunrise", "+001:00 sunrise"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], std::chrono::hours(1));
}

TEST_F(TimeStringConversion, SunoffsetTimeTestLongHour2) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"+000:00 sunrise", "+0010:00 sunrise"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], std::chrono::hours(10));
}

TEST_F(TimeStringConversion, SunoffsetSeconds) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"+00:00:01 sunrise",
                                         "-00:00:01 sunset",
                                         "+00:00:00 sunrise", "sunrise"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[0] - testSolarDay.sunrise, std::chrono::seconds(1));
  EXPECT_EQ(std::chrono::seconds(times[1]) -
                std::chrono::seconds(testSolarDay.sunset),
            -std::chrono::seconds(1));
  EXPECT_EQ(times[2], testSolarDay.sunrise);
  EXPECT_EQ(times[2], times[3]);
}

TEST_F(TimeStringConversion, SunoffsetSpaces) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes(
          {"    +00:00:01 sunrise", "-00:00:01     sunset",
           "+00:00:00 sunrise     ", "  sunrise  "},
          testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[0] - testSolarDay.sunrise, std::chrono::seconds(1));
  EXPECT_EQ(std::chrono::seconds(times[1]) -
                std::chrono::seconds(testSolarDay.sunset),
            -std::chrono::seconds(1));
  EXPECT_EQ(times[2], testSolarDay.sunrise);
  EXPECT_EQ(times[2], times[3]);
}

TEST_F(TimeStringConversion, BadSunriseSunsetStringsSymbol) {
  std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"?0:00 sunrise", "+00:00 sunset"},
                                        testSolarDay);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"1:00 sunrise", "+00:00 sunset"}, testSolarDay);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"!0:00 sunrise", "+00:00 sunset"}, testSolarDay);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"--0:00 sunrise", "+00:00 sunset"}, testSolarDay);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"++0:00 sunrise", "+00:00 sunset"}, testSolarDay);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"+-0:00 sunrise", "+00:00 sunset"}, testSolarDay);
  EXPECT_FALSE(timesOpt.has_value());
}

TEST_F(TimeStringConversion, BadSunriseSunsetStringsOffset) {
  std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"-0:71 sunrise", "+00:00 sunset"},
                                        testSolarDay);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"+01:000 sunrise", "+00:00 sunset"}, testSolarDay);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"+1x:000 sunrise", "+00:00 sunset"}, testSolarDay);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"+1:1000 sunrise", "+00:00 sunset"}, testSolarDay);
  EXPECT_FALSE(timesOpt.has_value());
}

TEST_F(TimeStringConversion, BadSunriseSunsetStringsRiseOrSet) {
  std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"-0:00 sun", "+00:00 sunset"},
                                        testSolarDay);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"+00:00 sunny", "+00:00 sunset"}, testSolarDay);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes({"+00:00 day", "+00:00 sunset"},
                                               testSolarDay);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"+00:00 night", "+00:00 sunset"}, testSolarDay);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"+00:00 sun rise", "+00:00 sunset"}, testSolarDay);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"+00:00 sun set", "+00:00 sunset"}, testSolarDay);
  EXPECT_FALSE(timesOpt.has_value());
}

TEST_F(TimeStringConversion, SunoffsetSpacing) {
  const std::optional<std::vector<TimeFromMidnight>> timesOpt =
      dynamic_paper::timeStringsToTimes({"- 2:20 sunrise", "+ 2:20 sunrise",
                                         "-      1:00             sunset"},
                                        testSolarDay);

  EXPECT_TRUE(timesOpt.has_value());
  assert(timesOpt.has_value());

  std::vector<TimeFromMidnight> times = timesOpt.value();

  EXPECT_EQ(times[0], testSolarDay.sunrise - std::chrono::hours(2) -
                          std::chrono::minutes(20));
  EXPECT_EQ(times[1], testSolarDay.sunrise + std::chrono::hours(2) +
                          std::chrono::minutes(20));
  EXPECT_EQ(times[2], testSolarDay.sunset - std::chrono::hours(1));
}
