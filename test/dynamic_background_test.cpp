/**
 * Test ability to show Dynamic Background Sets
 */

#include <array>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <tl/expected.hpp>

#include "background_test_setter.hpp"
#include "src/background_set_enums.hpp"
#include "src/background_setter.hpp"
#include "src/config.hpp"
#include "src/dynamic_background_set.hpp"
#include "src/time_from_midnight.hpp"
#include "src/time_util.hpp"
#include "src/transition_info.hpp"

using namespace dynamic_paper_test;
using namespace dynamic_paper;
using namespace testing;

// ===== Constants ===============

namespace {

// Files
constexpr std::string_view DATA_DIR = "./test_dir";
constexpr std::string_view CACHE_DIR = "./test_cache_dir";

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

  Config config = {
      std::filesystem::path(), std::nullopt, std::filesystem::path(CACHE_DIR),
      LocationInfo{.latitudeAndLongitude = {0, 0},
                   .useLatitudeAndLongitudeOverLocationSearch = false}};
  std::filesystem::path testDataDir = DATA_DIR;
};

// ===== Matcher ========
MATCHER_P(IsOneOf, examples, "") {
  const bool matches =
      std::find(examples.begin(), examples.end(), arg) != examples.end();
  *result_listener << "Is one of examples: " << matches;
  return matches;
}

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
  std::vector<TimeFromMidnight> times;
};
struct Backgrounds {
  std::vector<std::string> names{};
  BackgroundSetMode mode = BackgroundSetMode::Fill;
};

constexpr TimeFromMidnight time(std::string_view timeString) {
  return convertTimeStringToTimeFromMidnightUnchecked(timeString);
}

std::vector<TimeFromMidnight>
timesArray(const std::vector<std::string_view> &timeStrings) {
  std::vector<TimeFromMidnight> vec;
  vec.reserve(timeStrings.size());
  for (const auto &timeString : timeStrings) {
    vec.push_back(time(timeString));
  }
  return vec;
}

template <std::size_t N>
std::array<std::chrono::seconds, N>
testDynamicBackground(TestBackgroundSetterHistory &history,
                      const DynamicSetConfig setConfig,
                      const Backgrounds &backgrounds,
                      std::array<TimeFromMidnight, N> timesToUpdateWith) {
  auto setBackgroundFunc = [&history](const std::filesystem::path &imagePath,
                                      BackgroundSetMode mode) -> void {
    history.addEvent(SetEvent{imagePath, mode});
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
  const std::vector<TimeFromMidnight> times = timesArray({"01:00", "03:00"});

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
  EXPECT_EQ(waitTimesReturned[1], std::chrono::seconds(0));
  EXPECT_EQ(
      waitTimesReturned[2],
      std::chrono::seconds(std::chrono::hours(22) - std::chrono::seconds(1)));
  EXPECT_EQ(waitTimesReturned[3], std::chrono::seconds(0));

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

TEST_F(DynamicBackgroundTest, ShowBasicTile) {
  TestBackgroundSetterHistory history{};

  // Config options for the type of dynamic background set
  const BackgroundSetMode mode = BackgroundSetMode::Tile;
  const std::optional<TransitionInfo> transition(
      TransitionInfo(std::chrono::seconds(1), 2));
  const BackgroundSetOrder order = BackgroundSetOrder::Linear;

  const std::vector<std::string> imageNames = {"1.jpg", "2.jpg"};
  const std::vector<TimeFromMidnight> times = timesArray({"01:00", "03:00"});

  // Times to test with
  const std::array timesToCallUpdateBackground{time("01:00:00")};

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

  EXPECT_THAT(history.getHistory(),
              ElementsAre(
                  // show 1
                  SetEvent{.imagePath = data("1.jpg"), .mode = mode}));
}

TEST_F(DynamicBackgroundTest, ShowBasicScale) {
  TestBackgroundSetterHistory history{};

  // Config options for the type of dynamic background set
  const BackgroundSetMode mode = BackgroundSetMode::Scale;
  const std::optional<TransitionInfo> transition(
      TransitionInfo(std::chrono::seconds(1), 2));
  const BackgroundSetOrder order = BackgroundSetOrder::Linear;

  const std::vector<std::string> imageNames = {"1.jpg", "2.jpg"};
  const std::vector<TimeFromMidnight> times = timesArray({"01:00", "03:00"});

  // Times to test with
  const std::array timesToCallUpdateBackground{time("01:00:00")};

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

  EXPECT_THAT(history.getHistory(),
              ElementsAre(
                  // show 1
                  SetEvent{.imagePath = data("1.jpg"), .mode = mode}));
}

TEST_F(DynamicBackgroundTest, OneImage) {
  TestBackgroundSetterHistory history{};

  // Config options for the type of dynamic background set
  const BackgroundSetMode mode = BackgroundSetMode::Center;
  const std::optional<TransitionInfo> transition(
      TransitionInfo(std::chrono::seconds(1), 2));
  const BackgroundSetOrder order = BackgroundSetOrder::Linear;

  const std::vector<std::string> imageNames = {"1.jpg"};
  const std::vector<TimeFromMidnight> times = timesArray({"3:00"});

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
  EXPECT_EQ(waitTimesReturned[0], std::chrono::hours(24));
  EXPECT_EQ(waitTimesReturned[1],
            std::chrono::hours(24) - std::chrono::seconds(1));
  EXPECT_EQ(waitTimesReturned[2], std::chrono::seconds(1));

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
  const std::vector<TimeFromMidnight> times = {
      time("0:00"), time("1:00"), time("2:00"), time("3:00"), time("4:00")};

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
  EXPECT_EQ(waitTimesReturned[1], (std::chrono::seconds(0)));
  EXPECT_EQ(waitTimesReturned[2], (std::chrono::seconds(0)));
  EXPECT_EQ(waitTimesReturned[3], (std::chrono::seconds(0)));

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

TEST_F(DynamicBackgroundTest, MidnightTransition) {
  TestBackgroundSetterHistory history{};

  // Config options for the type of dynamic background set
  const BackgroundSetMode mode = BackgroundSetMode::Fill;
  const std::optional<TransitionInfo> transition(
      TransitionInfo(std::chrono::seconds(5), 5));
  const BackgroundSetOrder order = BackgroundSetOrder::Linear;

  const std::vector<std::string> imageNames = {"1.jpg", "2.jpg"};
  const std::vector<TimeFromMidnight> times = {time("0:00"), time("1:00")};

  // Times to test with
  const std::array timesToCallUpdateBackground{
      time("23:59:56"), time("00:00"), time("00:59:58"), time("1:00:04")};

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
  EXPECT_EQ(waitTimesReturned[0], std::chrono::seconds(0));
  EXPECT_EQ(waitTimesReturned[1],
            (std::chrono::hours(1) - std::chrono::seconds(5)));
  EXPECT_EQ(waitTimesReturned[2], (std::chrono::seconds(0)));
  EXPECT_EQ(waitTimesReturned[3],
            (std::chrono::hours(23) - std::chrono::seconds(4 + 5)));

  EXPECT_THAT(
      history.getHistory(),
      ElementsAre(
          // lerp 2->1
          SetEvent{.imagePath = cache("test_dir-2-1-16.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-2-1-33.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-2-1-50.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-2-1-66.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-2-1-83.jpg"), .mode = mode},
          // show 1
          SetEvent{.imagePath = data("1.jpg"), .mode = mode},
          // lerp 1->2
          SetEvent{.imagePath = cache("test_dir-1-2-16.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-1-2-33.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-1-2-50.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-1-2-66.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-1-2-83.jpg"), .mode = mode},
          // show 2
          SetEvent{.imagePath = data("2.jpg"), .mode = mode}));
}

TEST_F(DynamicBackgroundTest, RandomNoTransition) {
  TestBackgroundSetterHistory history{};

  // Config options for the type of dynamic background set
  const BackgroundSetMode mode = BackgroundSetMode::Fill;
  const std::optional<TransitionInfo> transition = std::nullopt;
  const BackgroundSetOrder order = BackgroundSetOrder::Random;

  const std::vector<std::string> imageNames = {"apple.jpg", "orange.jpg",
                                               "banana.jpg"};
  const std::vector<TimeFromMidnight> times = {time("4:00"), time("8:00"),
                                               time("12:00")};

  // Times to test with
  const std::array timesToCallUpdateBackground{time("3:59:55"), time("4:00"),
                                               time("8:00:02"), time("19:00")};

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
  EXPECT_EQ(waitTimesReturned[0], std::chrono::seconds(5));
  EXPECT_EQ(waitTimesReturned[1], std::chrono::hours(4));
  EXPECT_EQ(waitTimesReturned[2],
            (std::chrono::hours(4) - std::chrono::seconds(2)));
  EXPECT_EQ(waitTimesReturned[3], std::chrono::hours(9));

  const std::vector<SetEvent> possibleSetEvents = {
      SetEvent{.imagePath = data("apple.jpg"), .mode = mode},
      SetEvent{.imagePath = data("orange.jpg"), .mode = mode},
      SetEvent{.imagePath = data("banana.jpg"), .mode = mode}};

  EXPECT_THAT(history.getHistory(), ElementsAre(IsOneOf(possibleSetEvents),
                                                IsOneOf(possibleSetEvents),
                                                IsOneOf(possibleSetEvents),
                                                IsOneOf(possibleSetEvents)));
}

TEST_F(DynamicBackgroundTest, RandomTransition) {
  TestBackgroundSetterHistory history{};

  // Config options for the type of dynamic background set
  const BackgroundSetMode mode = BackgroundSetMode::Fill;
  const std::optional<TransitionInfo> transition(
      TransitionInfo(std::chrono::seconds(1), 2));
  const BackgroundSetOrder order = BackgroundSetOrder::Random;

  const std::vector<std::string> imageNames = {"apple.jpg", "orange.jpg",
                                               "banana.jpg"};
  const std::vector<TimeFromMidnight> times = {time("4:00"), time("8:00"),
                                               time("12:00")};

  // Times to test with
  const std::array timesToCallUpdateBackground{time("4:00"), time("7:59:59")};

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
            (std::chrono::hours(4) - std::chrono::seconds(1)));
  EXPECT_EQ(waitTimesReturned[1], std::chrono::seconds(0));

  const std::vector<SetEvent> possibleSetEvents = {
      SetEvent{.imagePath = data("apple.jpg"), .mode = mode},
      SetEvent{.imagePath = data("orange.jpg"), .mode = mode},
      SetEvent{.imagePath = data("banana.jpg"), .mode = mode}};
  const std::vector<SetEvent> possibleLerpEvents = {
      // apple -> orange
      SetEvent{.imagePath = cache("test_dir-apple-orange-33.jpg"),
               .mode = mode},
      SetEvent{.imagePath = cache("test_dir-apple-orange-66.jpg"),
               .mode = mode},
      // apple -> banana
      SetEvent{.imagePath = cache("test_dir-apple-banana-33.jpg"),
               .mode = mode},
      SetEvent{.imagePath = cache("test_dir-apple-banana-66.jpg"),
               .mode = mode},
      // orange -> apple
      SetEvent{.imagePath = cache("test_dir-orange-apple-33.jpg"),
               .mode = mode},
      SetEvent{.imagePath = cache("test_dir-orange-apple-66.jpg"),
               .mode = mode},
      // orange -> banana
      SetEvent{.imagePath = cache("test_dir-orange-banana-33.jpg"),
               .mode = mode},
      SetEvent{.imagePath = cache("test_dir-orange-banana-66.jpg"),
               .mode = mode},
      // banana -> apple
      SetEvent{.imagePath = cache("test_dir-banana-apple-33.jpg"),
               .mode = mode},
      SetEvent{.imagePath = cache("test_dir-banana-apple-66.jpg"),
               .mode = mode},
      // banana -> orange
      SetEvent{.imagePath = cache("test_dir-banana-orange-33.jpg"),
               .mode = mode},
      SetEvent{.imagePath = cache("test_dir-banana-orange-66.jpg"),
               .mode = mode},
  };

  EXPECT_THAT(history.getHistory(), ElementsAre(IsOneOf(possibleSetEvents),
                                                IsOneOf(possibleLerpEvents),
                                                IsOneOf(possibleLerpEvents)));
}

TEST_F(DynamicBackgroundTest, NoFileExtensionOnFirst) {
  TestBackgroundSetterHistory history{};

  // Config options for the type of dynamic background set
  const BackgroundSetMode mode = BackgroundSetMode::Fill;
  const std::optional<TransitionInfo> transition(
      TransitionInfo(std::chrono::seconds(1), 2));
  const BackgroundSetOrder order = BackgroundSetOrder::Linear;

  const std::vector<std::string> imageNames = {"start", "end.jpg"};
  const std::vector<TimeFromMidnight> times = {time("2:00"), time("4:00")};

  // Times to test with
  const std::array timesToCallUpdateBackground{time("2:00"), time("3:59:59"),
                                               time("4:00")};

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
            (std::chrono::hours(2) - std::chrono::seconds(1)));
  EXPECT_EQ(waitTimesReturned[1], std::chrono::seconds(0));
  EXPECT_EQ(waitTimesReturned[2],
            std::chrono::hours(22) - std::chrono::seconds(1));

  EXPECT_THAT(
      history.getHistory(),
      ElementsAre(SetEvent{.imagePath = data("start"), .mode = mode},
                  SetEvent{.imagePath = cache("test_dir-start-end-33.jpg"),
                           .mode = mode},
                  SetEvent{.imagePath = cache("test_dir-start-end-66.jpg"),
                           .mode = mode},
                  SetEvent{.imagePath = data("end.jpg"), .mode = mode}));
}

TEST_F(DynamicBackgroundTest, NoFileExtensionOnSecond) {
  TestBackgroundSetterHistory history{};

  // Config options for the type of dynamic background set
  const BackgroundSetMode mode = BackgroundSetMode::Fill;
  const std::optional<TransitionInfo> transition(
      TransitionInfo(std::chrono::seconds(1), 2));
  const BackgroundSetOrder order = BackgroundSetOrder::Linear;

  const std::vector<std::string> imageNames = {"start.png", "end"};
  const std::vector<TimeFromMidnight> times = {time("2:00"), time("4:00")};

  // Times to test with
  const std::array timesToCallUpdateBackground{time("2:00"), time("3:59:59"),
                                               time("4:00")};

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
            (std::chrono::hours(2) - std::chrono::seconds(1)));
  EXPECT_EQ(waitTimesReturned[1], std::chrono::seconds(0));
  EXPECT_EQ(waitTimesReturned[2],
            std::chrono::hours(22) - std::chrono::seconds(1));

  EXPECT_THAT(
      history.getHistory(),
      ElementsAre(SetEvent{.imagePath = data("start.png"), .mode = mode},
                  SetEvent{.imagePath = cache("test_dir-start-end-33.png"),
                           .mode = mode},
                  SetEvent{.imagePath = cache("test_dir-start-end-66.png"),
                           .mode = mode},
                  SetEvent{.imagePath = data("end"), .mode = mode}));
}

TEST_F(DynamicBackgroundTest, NoFileExtensionOnBoth) {
  TestBackgroundSetterHistory history{};

  // Config options for the type of dynamic background set
  const BackgroundSetMode mode = BackgroundSetMode::Fill;
  const std::optional<TransitionInfo> transition(
      TransitionInfo(std::chrono::seconds(1), 2));
  const BackgroundSetOrder order = BackgroundSetOrder::Linear;

  const std::vector<std::string> imageNames = {"start", "end"};
  const std::vector<TimeFromMidnight> times = {time("2:00"), time("4:00")};

  // Times to test with
  const std::array timesToCallUpdateBackground{time("2:00"), time("3:59:59"),
                                               time("4:00")};

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
            (std::chrono::hours(2) - std::chrono::seconds(1)));
  EXPECT_EQ(waitTimesReturned[1], std::chrono::seconds(0));
  EXPECT_EQ(waitTimesReturned[2],
            std::chrono::hours(22) - std::chrono::seconds(1));

  EXPECT_THAT(
      history.getHistory(),
      ElementsAre(
          SetEvent{.imagePath = data("start"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-start-end-33"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-start-end-66"), .mode = mode},
          SetEvent{.imagePath = data("end"), .mode = mode}));
}

TEST_F(DynamicBackgroundTest, FileExtensionsDiffer) {
  TestBackgroundSetterHistory history{};

  // Config options for the type of dynamic background set
  const BackgroundSetMode mode = BackgroundSetMode::Fill;
  const std::optional<TransitionInfo> transition(
      TransitionInfo(std::chrono::seconds(1), 2));
  const BackgroundSetOrder order = BackgroundSetOrder::Linear;

  const std::vector<std::string> imageNames = {"start.png", "end.jpg"};
  const std::vector<TimeFromMidnight> times = {time("2:00"), time("4:00")};

  // Times to test with
  const std::array timesToCallUpdateBackground{time("2:00"), time("3:59:59"),
                                               time("4:00")};

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
            (std::chrono::hours(2) - std::chrono::seconds(1)));
  EXPECT_EQ(waitTimesReturned[1], std::chrono::seconds(0));
  EXPECT_EQ(waitTimesReturned[2],
            std::chrono::hours(22) - std::chrono::seconds(1));

  EXPECT_THAT(
      history.getHistory(),
      ElementsAre(SetEvent{.imagePath = data("start.png"), .mode = mode},
                  SetEvent{.imagePath = cache("test_dir-start-end-33.png"),
                           .mode = mode},
                  SetEvent{.imagePath = cache("test_dir-start-end-66.png"),
                           .mode = mode},
                  SetEvent{.imagePath = data("end.jpg"), .mode = mode}));
}

TEST_F(DynamicBackgroundTest, OverlappingTransitions) {
  TestBackgroundSetterHistory history{};

  // Config options for the type of dynamic background set
  const BackgroundSetMode mode = BackgroundSetMode::Fill;
  const std::optional<TransitionInfo> transition(
      TransitionInfo(std::chrono::seconds(10), 2));
  const BackgroundSetOrder order = BackgroundSetOrder::Linear;

  const std::vector<std::string> imageNames = {"1.jpg", "2.jpg", "3.jpg"};
  const std::vector<TimeFromMidnight> times = {
      time("23:59:55"), time("00:00:00"), time("0:00:08")};

  // Times to test with
  const std::array timesToCallUpdateBackground{
      time("23:59:55"), time("00:00:00"), time("00:00:02"), time("00:00:12")};

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
  EXPECT_EQ(waitTimesReturned[0], std::chrono::seconds(1));
  EXPECT_EQ(waitTimesReturned[1], std::chrono::seconds(1));
  EXPECT_EQ(waitTimesReturned[2], std::chrono::seconds(0));
  EXPECT_EQ(waitTimesReturned[3],
            (std::chrono::hours(24) - std::chrono::seconds(12 + 15)));

  EXPECT_THAT(
      history.getHistory(),
      ElementsAre(
          // show 1
          SetEvent{.imagePath = data("1.jpg"), .mode = mode},
          // show 2
          SetEvent{.imagePath = data("2.jpg"), .mode = mode},
          // lerp 2->3
          SetEvent{.imagePath = cache("test_dir-2-3-33.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-2-3-66.jpg"), .mode = mode},
          // show 3
          SetEvent{.imagePath = data("3.jpg"), .mode = mode}));
}

TEST_F(DynamicBackgroundTest, OverlappingTransitions2) {
  TestBackgroundSetterHistory history{};

  // Config options for the type of dynamic background set
  const BackgroundSetMode mode = BackgroundSetMode::Fill;
  const std::optional<TransitionInfo> transition(
      TransitionInfo(std::chrono::hours(10), 2));
  const BackgroundSetOrder order = BackgroundSetOrder::Linear;

  const std::vector<std::string> imageNames = {"1.jpg", "2.jpg", "3.jpg"};
  const std::vector<TimeFromMidnight> times = {time("20:00"), time("22:00"),
                                               time("1:00")};

  // Times to test with
  const std::array timesToCallUpdateBackground{time("20:00"), time("20:00:10"),
                                               time("22:00"), time("00:00"),
                                               time("1:00"),  time("11:00")};

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
  EXPECT_EQ(waitTimesReturned[0], std::chrono::seconds(1));
  EXPECT_EQ(waitTimesReturned[1], std::chrono::seconds(0));
  EXPECT_EQ(waitTimesReturned[2], std::chrono::seconds(1));
  EXPECT_EQ(waitTimesReturned[3], std::chrono::seconds(0));
  EXPECT_EQ(waitTimesReturned[4], std::chrono::hours(9));
  EXPECT_EQ(waitTimesReturned[5], std::chrono::seconds(0));

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
          // lerp 2->3
          SetEvent{.imagePath = cache("test_dir-2-3-33.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-2-3-66.jpg"), .mode = mode},
          // show 3
          SetEvent{.imagePath = data("3.jpg"), .mode = mode},
          // show 3->1
          SetEvent{.imagePath = cache("test_dir-3-1-33.jpg"), .mode = mode},
          SetEvent{.imagePath = cache("test_dir-3-1-66.jpg"), .mode = mode}));
}

/**
 * Test when transitions start at the exact same time.
 *
 * This is forbidden, but should remove events to stop this
 * The policy is that overlapping transition events should both be removed.
 * If a transition and set event overlap, keep the set event.
 * If 2 set events overlap, keep the set event that came up last in the parsing
 * of the config list.
 */
TEST_F(DynamicBackgroundTest, SameTimeTransitions) {
  TestBackgroundSetterHistory history{};

  // Config options for the type of dynamic background set
  const BackgroundSetMode mode = BackgroundSetMode::Fill;
  const std::optional<TransitionInfo> transition(
      TransitionInfo(std::chrono::seconds(1), 2));
  const BackgroundSetOrder order = BackgroundSetOrder::Linear;

  const std::vector<std::string> imageNames = {"1.jpg", "2.jpg", "3.jpg",
                                               "4.jpg"};
  const std::vector<TimeFromMidnight> times = {
      time("1:00"), time("1:00"),    // 2 lerp , 2 set
      time("2:00"), time("2:00:01"), // 1 lerp and 1 set, but no 0 length
                                     // transitions will remove one
  };

  // Times to test with
  const std::array timesToCallUpdateBackground{time("0:59:59"), time("1:00"),
                                               time("2:00")};

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
  EXPECT_EQ(waitTimesReturned[0], std::chrono::seconds(1));
  EXPECT_EQ(waitTimesReturned[1],
            std::chrono::hours(1) - std::chrono::seconds(1));
  EXPECT_EQ(waitTimesReturned[2], std::chrono::seconds(1));

  EXPECT_THAT(history.getHistory(),
              ElementsAre(
                  // show 4 (both transitions were removed)
                  SetEvent{.imagePath = data("4.jpg"), .mode = mode},
                  // show 2 (1 overlaps is is removed)
                  SetEvent{.imagePath = data("2.jpg"), .mode = mode},
                  // show 3 (transition is removed for the set event)
                  SetEvent{.imagePath = data("3.jpg"), .mode = mode}));
}
