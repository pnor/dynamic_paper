/**
 * Test how TimeFromMidnight represents times
 */

#include <chrono>

#include <gtest/gtest.h>

#include "src/time_from_midnight.hpp"
#include "src/time_util.hpp"

using namespace dynamic_paper;
using namespace testing;

TEST(TimeFromMidnight, BasicConstruction) {
  const TimeFromMidnight time1(std::chrono::minutes(3));
  const TimeFromMidnight time2 = std::chrono::seconds(1);
  const TimeFromMidnight time3(std::chrono::days(1));
  const TimeFromMidnight time4 = std::chrono::hours(1);

  EXPECT_EQ(time1, time1);
  EXPECT_EQ(time2, time2);
  EXPECT_EQ(time3, time3);
  EXPECT_EQ(time4, time4);
}

TEST(TimeFromMidnight, ComparisonEquality) {
  const TimeFromMidnight oneHour = std::chrono::hours(1);
  const TimeFromMidnight alsoAnHour = std::chrono::minutes(60);
  const TimeFromMidnight yetAnotherHour = std::chrono::seconds(60 * 60);
  const TimeFromMidnight finalHour(std::chrono::minutes(30) +
                                   std::chrono::seconds(60 * 30));

  EXPECT_EQ(oneHour, alsoAnHour);
  EXPECT_EQ(alsoAnHour, yetAnotherHour);
  EXPECT_EQ(finalHour, oneHour);
  EXPECT_EQ(finalHour, alsoAnHour);
  EXPECT_EQ(finalHour, finalHour);
}

TEST(TimeFromMidnight, ComparisonInequality) {
  const TimeFromMidnight one = std::chrono::hours(1);
  const TimeFromMidnight two = std::chrono::hours(2);
  const TimeFromMidnight three = std::chrono::hours(3);
  const TimeFromMidnight four = std::chrono::hours(4);

  EXPECT_LT(one, two);
  EXPECT_GT(two, one);

  EXPECT_LT(three, four);
  EXPECT_GT(four, three);

  EXPECT_LT(one, four);
  EXPECT_GT(four, one);
}

TEST(TimeFromMidnight, FromStringTimes) {
  const TimeFromMidnight beginningofDay =
      convertTimeStringToTimeFromMidnightUnchecked("00:00");
  const TimeFromMidnight oneSecond =
      convertTimeStringToTimeFromMidnightUnchecked("00:00:01");
  const TimeFromMidnight noon =
      convertTimeStringToTimeFromMidnightUnchecked("12:00");
  const TimeFromMidnight endOfDay =
      convertTimeStringToTimeFromMidnightUnchecked("23:59:59");

  EXPECT_EQ(beginningofDay, TimeFromMidnight(std::chrono::seconds(0)));
  EXPECT_EQ(oneSecond, TimeFromMidnight(std::chrono::seconds(1)));
  EXPECT_EQ(noon, TimeFromMidnight(std::chrono::hours(12)));
  EXPECT_EQ(endOfDay,
            TimeFromMidnight(std::chrono::hours(24) - std::chrono::seconds(1)));
}
