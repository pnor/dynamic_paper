
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

#include <gtest/gtest.h>

#include "constants.hpp"

using namespace dynamic_paper;
using namespace dynamic_paper_test;

// ===== Parsing HH:MM =====

TEST(TimeStringConversion, RawTimeTest) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"00:00", "01:00"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], ONE_HOUR);
}

TEST(TimeStringConversion, RawTimeTest2) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"01:23", "04:55"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], (ONE_HOUR * 3) + (ONE_MINUTE * 32));
}

TEST(TimeStringConversion, RawTimeTest3) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"00:00", "23:59"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], (ONE_HOUR * 23) + (ONE_MINUTE * 59));
}

TEST(TimeStringConversion, RawTimeTest4) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"11:11", "22:55"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], (ONE_HOUR * 11) + (ONE_MINUTE * 44));
}

TEST(TimeStringConversion, RawTimeTest5) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"0:00", "1:00"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], ONE_HOUR);
}

TEST(TimeStringConversion, BadRawTimeTestTooManyMinutes) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"01:90", "03:00"},
                                        testSunriseAndSunsetTimes);

  EXPECT_FALSE(timesOpt.has_value());
}

TEST(TimeStringConversion, BadRawTimeTestTooLongString) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"001:90", "03:00"},
                                        testSunriseAndSunsetTimes);

  EXPECT_FALSE(timesOpt.has_value());
}

TEST(TimeStringConversion, BadRawTimeTestTooLongString2) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"130:900", "03:00"},
                                        testSunriseAndSunsetTimes);

  EXPECT_FALSE(timesOpt.has_value());
}

TEST(TimeStringConversion, BadRawTimeTestAtLeastOneIsBad) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes(
          {"00:00", "01:00", "2:00", "0003:00000", "4:00"},
          testSunriseAndSunsetTimes);

  EXPECT_FALSE(timesOpt.has_value());
}

// ===== Parsing +/-HH:MM sunrise/sunset =====

TEST(TimeStringConversion, SunoffsetTimeTestSunrise) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"+00:00 sunrise", "+01:00 sunrise"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], ONE_HOUR);
}

TEST(TimeStringConversion, SunoffsetTimeTestSunrise2) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"-00:01 sunrise", "-00:00 sunrise"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], ONE_MINUTE);
}

TEST(TimeStringConversion, SunoffsetTimeTestSunrise3) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"sunrise", "+01:30 sunrise"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], ONE_HOUR + (30 * ONE_MINUTE));
}

TEST(TimeStringConversion, SunoffsetTimeTestSunrise4) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"-2:20 sunrise", "+2:20 sunrise"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], 2 * (((2 * ONE_HOUR) + (20 * ONE_MINUTE))));
}

TEST(TimeStringConversion, SunoffsetTimeTestSunset) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"+00:00 sunset", "+01:00 sunset"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], ONE_HOUR);
}

TEST(TimeStringConversion, SunoffsetTimeTestSunset2) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"-00:01 sunset", "-00:00 sunset"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], ONE_MINUTE);
}

TEST(TimeStringConversion, SunoffsetTimeTestSunset3) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"sunset", "+01:30 sunset"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], ONE_HOUR + (30 * ONE_MINUTE));
}

TEST(TimeStringConversion, SunoffsetTimeTestSunset4) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"-2:20 sunset", "+2:20 sunset"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], 2 * (((2 * ONE_HOUR) + (20 * ONE_MINUTE))));
}

TEST(TimeStringConversion, SunoffsetTimeTestSunsetAndSunrise) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"sunrise", "sunset"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], SUNRISE_SUNSET_GAP);
}

TEST(TimeStringConversion, SunoffsetTimeTestSunsetAndSunrise2) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"sunrise", "+3:00 sunset"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], SUNRISE_SUNSET_GAP + (3 * ONE_HOUR));
}

TEST(TimeStringConversion, SunoffsetTimeTestSunsetAndSunrise3) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"-10:11 sunrise", "+1:22 sunset"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], SUNRISE_SUNSET_GAP +
                                     ((10 * ONE_HOUR) + (11 * ONE_MINUTE)) +
                                     ((1 * ONE_HOUR) + (22 * ONE_MINUTE)));
}

TEST(TimeStringConversion, SunriseCapitalization) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"SUNRISE", "+1:00 sunrise"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());
  std::vector<time_t> times = timesOpt.value();
  EXPECT_EQ(times[1] - times[0], ONE_HOUR);

  timesOpt = dynamic_paper::timeStringsToTimes({"sunrise", "+1:00 SUNRISE"},
                                               testSunriseAndSunsetTimes);
  EXPECT_TRUE(timesOpt.has_value());
  times = timesOpt.value();
  EXPECT_EQ(times[1] - times[0], ONE_HOUR);

  timesOpt = dynamic_paper::timeStringsToTimes({"SUNRISE", "+1:00 SUNRISE"},
                                               testSunriseAndSunsetTimes);
  EXPECT_TRUE(timesOpt.has_value());
  times = timesOpt.value();
  EXPECT_EQ(times[1] - times[0], ONE_HOUR);

  timesOpt = dynamic_paper::timeStringsToTimes({"sUnRiSe", "+1:00 sunRISE"},
                                               testSunriseAndSunsetTimes);
  EXPECT_TRUE(timesOpt.has_value());
  times = timesOpt.value();
  EXPECT_EQ(times[1] - times[0], ONE_HOUR);
}

TEST(TimeStringConversion, SunsetCapitalization) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"SUNSET", "+1:00 sunset"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());
  std::vector<time_t> times = timesOpt.value();
  EXPECT_EQ(times[1] - times[0], ONE_HOUR);

  timesOpt = dynamic_paper::timeStringsToTimes({"sunset", "+1:00 SUNSET"},
                                               testSunriseAndSunsetTimes);
  EXPECT_TRUE(timesOpt.has_value());
  times = timesOpt.value();
  EXPECT_EQ(times[1] - times[0], ONE_HOUR);

  timesOpt = dynamic_paper::timeStringsToTimes({"SUNSET", "+1:00 SUNSET"},
                                               testSunriseAndSunsetTimes);
  EXPECT_TRUE(timesOpt.has_value());
  times = timesOpt.value();
  EXPECT_EQ(times[1] - times[0], ONE_HOUR);

  timesOpt = dynamic_paper::timeStringsToTimes({"sunset", "+1:00 sunset"},
                                               testSunriseAndSunsetTimes);
  EXPECT_TRUE(timesOpt.has_value());
  times = timesOpt.value();
  EXPECT_EQ(times[1] - times[0], ONE_HOUR);
}

TEST(TimeStringConversion, SunriseEquivalence) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"sunrise", "sunrise"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], 0);
}

TEST(TimeStringConversion, SunriseEquivalence2) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"+1:00 sunrise", "+1:00 sunrise"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], 0);
}

TEST(TimeStringConversion, SunriseEquivalence3) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"-1:00 sunrise", "-1:00 sunrise"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], 0);
}

TEST(TimeStringConversion, SunsetEquivalence) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"sunset", "sunset"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], 0);
}

TEST(TimeStringConversion, SunsetEquivalence2) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"-1:00 sunset", "-01:00 sunset"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], 0);
}

TEST(TimeStringConversion, SunsetEquivalence3) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"+1:00 sunset", "+01:00 sunset"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], 0);
}

TEST(TimeStringConversion, SunoffsetTimeTestLongHour) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"+000:00 sunrise", "+001:00 sunrise"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], ONE_HOUR);
}

TEST(TimeStringConversion, SunoffsetTimeTestLongHour2) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"+000:00 sunrise", "+100:00 sunrise"},
                                        testSunriseAndSunsetTimes);

  EXPECT_TRUE(timesOpt.has_value());

  std::vector<time_t> times = timesOpt.value();

  EXPECT_EQ(times[1] - times[0], ONE_HOUR * 100);
}

TEST(TimeStringConversion, BadSunriseSunsetStringsSymbol) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"?0:00 sunrise", "+00:00 sunset"},
                                        testSunriseAndSunsetTimes);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"1:00 sunrise", "+00:00 sunset"}, testSunriseAndSunsetTimes);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"!0:00 sunrise", "+00:00 sunset"}, testSunriseAndSunsetTimes);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"--0:00 sunrise", "+00:00 sunset"}, testSunriseAndSunsetTimes);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"++0:00 sunrise", "+00:00 sunset"}, testSunriseAndSunsetTimes);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"+-0:00 sunrise", "+00:00 sunset"}, testSunriseAndSunsetTimes);
  EXPECT_FALSE(timesOpt.has_value());
}

TEST(TimeStringConversion, BadSunriseSunsetStringsOffset) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"-0:71 sunrise", "+00:00 sunset"},
                                        testSunriseAndSunsetTimes);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"+01:000 sunrise", "+00:00 sunset"}, testSunriseAndSunsetTimes);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"+1x:000 sunrise", "+00:00 sunset"}, testSunriseAndSunsetTimes);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"+1:1000 sunrise", "+00:00 sunset"}, testSunriseAndSunsetTimes);
  EXPECT_FALSE(timesOpt.has_value());
}

TEST(TimeStringConversion, BadSunriseSunsetStringsRiseOrSet) {
  std::optional<std::vector<time_t>> timesOpt =
      dynamic_paper::timeStringsToTimes({"-0:00 sun", "+00:00 sunset"},
                                        testSunriseAndSunsetTimes);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"+00:00 sunny", "+00:00 sunset"}, testSunriseAndSunsetTimes);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes({"+00:00 day", "+00:00 sunset"},
                                               testSunriseAndSunsetTimes);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"+00:00 night", "+00:00 sunset"}, testSunriseAndSunsetTimes);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"+00:00 sun rise", "+00:00 sunset"}, testSunriseAndSunsetTimes);
  EXPECT_FALSE(timesOpt.has_value());

  timesOpt = dynamic_paper::timeStringsToTimes(
      {"+00:00 sun set", "+00:00 sunset"}, testSunriseAndSunsetTimes);
  EXPECT_FALSE(timesOpt.has_value());
}
