/**
 * Test ability to show Dynamic Background Sets
 */

#include <cassert>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <tl/expected.hpp>

#include "background_test_setter.hpp"
#include "constants.hpp"
#include "src/background_setter.hpp"
#include "src/config.hpp"
#include "src/dynamic_background_set.hpp"
#include "src/static_background_set.hpp"

using namespace dynamic_paper_test;
using namespace dynamic_paper;
using namespace testing;

// ===== Constants ===============

namespace {

// Files
constexpr std::string_view DATA_DIR = "./test_dir";
constexpr std::string_view CACHE_DIR = "./test_cache_dir";

// `time_t` times
constexpr int HOURS_IN_DAY = 24;
constexpr std::array<std::string_view, HOURS_IN_DAY> HOURS_STRINGS = {
    "00:00", "01:00", "02:00", "03:00", "04:00", "05:00", "06:00", "07:00",
    "08:00", "09:00", "10:00", "11:00", "12:00", "13:00", "14:00", "15:00",
    "16:00", "17:00", "18:00", "19:00", "20:00", "21:00", "22:00", "23:00"};

consteval std::array<time_t, HOURS_IN_DAY> calculateTimeArray() {
  std::array<time_t, HOURS_IN_DAY> arr{};

  for (int i = 0; i < HOURS_IN_DAY; i++) {
    std::optional<time_t> time =
        convertRawTimeStringToTimeOffset(HOURS_STRINGS.at(i));
    logAssert(time.has_value(), "time must have value");
    arr.at(i) = time.value();
  }

  return arr;
}

constexpr std::array<time_t, HOURS_IN_DAY> TIME_ARRAY = calculateTimeArray();

// Test Function Classes

class TestFilesystemHandler {
public:
  static bool
  createDirectoryIfDoesntExist(const std::filesystem::path & /*unused*/) {
    return true;
  }
  static bool createFileIfDoesntExist(const std::filesystem::path & /*unused*/,
                                      std::string_view /*unused*/) {
    return true;
  }
  static bool exists(const std::filesystem::path & /* unused */) {
    return true;
  }
};

class TestCompositeImages {
public:
  static tl::expected<std::filesystem::path, CompositeImageError>
  getCompositedImage(const std::filesystem::path &commonImageDirectory,
                     const std::string &startImageName,
                     const std::string &endImageName,
                     const std::filesystem::path &cacheDirectory,
                     unsigned int percentage) {
    return pathForCompositeImage(commonImageDirectory, startImageName,
                                 endImageName, percentage, cacheDirectory);
  }
};

} // namespace

// ===== Test Fixture ===============

class DynamicBackgroundTest : public testing::Test {
public:
  void SetUp() override {}

  Config config = {std::filesystem::path(),
                   {BackgroundSetterMethodWallUtils()},
                   SunEventPollerMethod::Dummy,
                   std::nullopt,
                   std::filesystem::path(CACHE_DIR)};
  std::filesystem::path testDataDir = DATA_DIR;
};

// ===== Tests ===============

TEST_F(DynamicBackgroundTest, ShowBasic) {
  TestBackgroundSetterHistory history{};

  const BackgroundSetMode mode = BackgroundSetMode::Center;

  const std::optional<TransitionInfo> transition(
      TransitionInfo{.duration = std::chrono::seconds(0), .steps = 2});

  const BackgroundSetOrder order = BackgroundSetOrder::Linear;

  const std::vector<std::string> imageNames = {"1.jpg", "2.jpg"};

  const std::vector<time_t> times = {TIME_ARRAY[1], TIME_ARRAY[3]};

  // ;;;
  const DynamicBackgroundTest &test = *this;
  auto setBackgroundFunc = [&history](const std::filesystem::path &imagePath,
                                      BackgroundSetMode mode,
                                      const BackgroundSetterMethod &)
      -> tl::expected<void, BackgroundError> {
    history.addEvent(SetEvent{imagePath, mode});
    return {};
  };

  const DynamicBackgroundData dynamicData(test.testDataDir, mode, transition,
                                          order, imageNames, times);

  using LambdaType = decltype(setBackgroundFunc);
  const std::chrono::seconds firstWaitTime =
      dynamicData.updateBackground<LambdaType, TestFilesystemHandler,
                                   TestCompositeImages>(
          TIME_ARRAY[1], config, std::forward<LambdaType>(setBackgroundFunc));

  const std::chrono::seconds secondWaitTime =
      dynamicData.updateBackground(TIME_ARRAY[3], config, setBackgroundFunc);

  EXPECT_EQ(firstWaitTime, std::chrono::seconds(ONE_HOUR));
  EXPECT_EQ(secondWaitTime, std::chrono::seconds(ONE_HOUR * 22));
}
