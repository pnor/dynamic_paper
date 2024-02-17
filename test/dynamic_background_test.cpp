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

// ===== Helper Function ===============

namespace {

std::filesystem::path cache(const std::string_view name) {
  return std::filesystem::path(CACHE_DIR) / name;
}

std::filesystem::path data(const std::string_view name) {
  return std::filesystem::path(DATA_DIR) / name;
}

struct DynamicSetConfig {
  std::filesystem::path dataDir;
  Config config;
  std::optional<TransitionInfo> transition;
  BackgroundSetOrder order;
  std::vector<time_t> times;
};
struct Backgrounds {
  std::vector<std::string> names;
  BackgroundSetMode mode;
};

constexpr time_t time(std::string_view timeString) {
  std::optional<time_t> optTime = convertRawTimeStringToTimeOffset(timeString);
  logAssert(optTime.has_value(), "time has value");
  return optTime.value();
}

std::vector<time_t>
timesArray(const std::vector<std::string_view> &timeStrings) {
  std::vector<time_t> vec;
  vec.reserve(timeStrings.size());
  for (const auto &timeString : timeStrings) {
    vec.push_back(time(timeString));
  }
  return vec;
}

template <size_t N>
std::array<std::chrono::seconds, N> testDynamicBackground(
    TestBackgroundSetterHistory &history, const DynamicSetConfig setConfig,
    const Backgrounds &backgrounds, std::array<time_t, N> timesToUpdateWith) {
  auto setBackgroundFunc = [&history](const std::filesystem::path &imagePath,
                                      BackgroundSetMode mode,
                                      const BackgroundSetterMethod &)
      -> tl::expected<void, BackgroundError> {
    history.addEvent(SetEvent{imagePath, mode});
    return {};
  };

  using LambdaType = decltype(setBackgroundFunc);

  const DynamicBackgroundData dynamicData(setConfig.dataDir, backgrounds.mode,
                                          setConfig.transition, setConfig.order,
                                          backgrounds.names, setConfig.times);

  std::array<std::chrono::seconds, N> waitTimes{};

  for (long unsigned int i = 0; i < N; i++) {
    waitTimes.at(i) =
        dynamicData.updateBackground<LambdaType, TestFilesystemHandler,
                                     TestCompositeImages>(
            timesToUpdateWith.at(i), setConfig.config,
            std::forward<LambdaType>(setBackgroundFunc));
  }

  return waitTimes;
}

} // namespace

// ===== Tests ===============

TEST_F(DynamicBackgroundTest, ShowBasic) {
  TestBackgroundSetterHistory history{};

  // Config options for the type of dynamic background set
  const BackgroundSetMode mode = BackgroundSetMode::Center;
  const std::optional<TransitionInfo> transition(
      TransitionInfo(std::chrono::seconds(1), 2));
  const BackgroundSetOrder order = BackgroundSetOrder::Linear;

  const std::vector<std::string> imageNames = {"1.jpg", "2.jpg"};
  const std::vector<time_t> times = timesArray({"01:00", "03:00"});

  // Times to test with
  const std::array timesToCallUpdateBackground{
      time("01:00:00"), time("02:59:59"), time("03:00:00"), time("00:59:59")};

  // Do the testing
  const std::array<std::chrono::seconds, timesToCallUpdateBackground.size()>
      waitTimesReturned =
          testDynamicBackground(history,
                                {.dataDir = this->testDataDir,
                                 .config = this->config,
                                 .transition = transition,
                                 .order = order,
                                 .times = times},
                                {.names = imageNames, .mode = mode},
                                timesToCallUpdateBackground);

  // Expectations
  EXPECT_EQ(
      waitTimesReturned[0],
      std::chrono::seconds(std::chrono::hours(2) - std::chrono::seconds(1)));
  EXPECT_EQ(waitTimesReturned[1], std::chrono::seconds(1));
  EXPECT_EQ(
      waitTimesReturned[2],
      std::chrono::seconds(std::chrono::hours(22) - std::chrono::seconds(1)));
  EXPECT_EQ(waitTimesReturned[3], std::chrono::seconds(1));

  EXPECT_THAT(
      history.getHistory(),
      ElementsAre(
          // show 1
          SetEvent{.imagePath = data("1.jpg"), .mode = mode},
          // lerp 1->2
          SetEvent{.imagePath = cache("test_dir-1-2-33.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-1-2-66.jpg"), .mode = mode},
          // show 2
          SetEvent{.imagePath = data("2.jpg"), .mode = mode},
          // lerp 2->1
          SetEvent{.imagePath = cache("test_dir-2-1-33.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-2-1-66.jpg"), .mode = mode}));
}

TEST_F(DynamicBackgroundTest, OneImage) {
  TestBackgroundSetterHistory history{};

  // Config options for the type of dynamic background set
  const BackgroundSetMode mode = BackgroundSetMode::Center;
  const std::optional<TransitionInfo> transition(
      TransitionInfo(std::chrono::seconds(1), 2));
  const BackgroundSetOrder order = BackgroundSetOrder::Linear;

  const std::vector<std::string> imageNames = {"1.jpg"};
  const std::vector<time_t> times = timesArray({"3:00"});

  // Times to test with
  const std::array timesToCallUpdateBackground{time("3:00:00"), time("3:00:01"),
                                               time("2:59:59")};

  // Do the testing
  const std::array<std::chrono::seconds, timesToCallUpdateBackground.size()>
      waitTimesReturned =
          testDynamicBackground(history,
                                {.dataDir = this->testDataDir,
                                 .config = this->config,
                                 .transition = transition,
                                 .order = order,
                                 .times = times},
                                {.names = imageNames, .mode = mode},
                                timesToCallUpdateBackground);

  // Expectations
  EXPECT_EQ(waitTimesReturned[0], std::chrono::seconds(TWENTY_FOUR_HOURS));
  EXPECT_EQ(waitTimesReturned[1],
            std::chrono::seconds(TWENTY_FOUR_HOURS - ONE_SECOND));
  EXPECT_EQ(waitTimesReturned[2], std::chrono::seconds(ONE_SECOND));

  EXPECT_THAT(history.getHistory(),
              ElementsAre(
                  // show 1
                  SetEvent{.imagePath = data("1.jpg"), .mode = mode},
                  // show 1
                  SetEvent{.imagePath = data("1.jpg"), .mode = mode},
                  // show 1
                  SetEvent{.imagePath = data("1.jpg"), .mode = mode}));
}

TEST_F(DynamicBackgroundTest, MoreTransitions) {
  TestBackgroundSetterHistory history{};

  // Config options for the type of dynamic background set
  const BackgroundSetMode mode = BackgroundSetMode::Fill;
  const std::optional<TransitionInfo> transition(
      TransitionInfo(std::chrono::seconds(5), 5));
  const BackgroundSetOrder order = BackgroundSetOrder::Linear;

  const std::vector<std::string> imageNames = {"1.jpg", "2.jpg", "3.jpg",
                                               "4.jpg", "5.jpg"};
  const std::vector<time_t> times = {time("0:00"), time("1:00"), time("2:00"),
                                     time("3:00"), time("4:00")};

  // Times to test with
  const std::array timesToCallUpdateBackground{
      time("00:00:00"), time("00:59:55"), time("03:59:58"), time("23:59:56")};

  // Do the testing
  const std::array<std::chrono::seconds, timesToCallUpdateBackground.size()>
      waitTimesReturned =
          testDynamicBackground(history,
                                {.dataDir = this->testDataDir,
                                 .config = this->config,
                                 .transition = transition,
                                 .order = order,
                                 .times = times},
                                {.names = imageNames, .mode = mode},
                                timesToCallUpdateBackground);

  // Expectations
  EXPECT_EQ(waitTimesReturned[0],
            (std::chrono::hours(1) - std::chrono::seconds(5)));
  EXPECT_EQ(waitTimesReturned[1], (std::chrono::seconds(5)));
  EXPECT_EQ(waitTimesReturned[2], (std::chrono::seconds(2)));
  EXPECT_EQ(waitTimesReturned[3], (std::chrono::seconds(4)));

  EXPECT_THAT(
      history.getHistory(),
      ElementsAre(
          // show 1
          SetEvent{.imagePath = data("1.jpg"), .mode = mode},
          // lerp 1->2
          SetEvent{.imagePath = cache("test_dir-1-2-16.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-1-2-33.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-1-2-50.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-1-2-66.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-1-2-83.jpg"), .mode = mode},
          // lerp 4->5
          SetEvent{.imagePath = cache("test_dir-4-5-16.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-4-5-33.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-4-5-50.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-4-5-66.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-4-5-83.jpg"), .mode = mode},
          // lerp 5->1
          SetEvent{.imagePath = cache("test_dir-5-1-16.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-5-1-33.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-5-1-50.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-5-1-66.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-5-1-83.jpg"), .mode = mode}));
}

// TODO transition on midnight
// TODO random no transition
// TODO random with transition
// TODO no file extension on first
// TODO no file extension on second

// TODO transition that would overlap a previous event (long transitions)
