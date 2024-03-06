/**
 *   Test the ability to get the time
 */

#include <gtest/gtest.h>

#include "src/time_util.hpp"

TEST(CurrentTime, CurrentTime) {
  const std::chrono::seconds before = dynamic_paper::getCurrentTime();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  const std::chrono::seconds after = dynamic_paper::getCurrentTime();

  const std::chrono::seconds gap =
      std::chrono::seconds(after) - std::chrono::seconds(before);

  if (gap < std::chrono::seconds(0)) { // ran test close to midnight
    EXPECT_GT(before, after);
    EXPECT_EQ(gap, std::chrono::hours(24) - std::chrono::seconds(1));
  } else {
    EXPECT_LT(before, after);
    EXPECT_LE(after - before, std::chrono::seconds(1));
    EXPECT_NE(after - before, std::chrono::seconds(0));
  }
}
