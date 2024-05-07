/**
 * Test ability to show Static Backgrounds Sets
 */

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <tl/expected.hpp>

#include "background_test_setter.hpp"
#include "src/background_set_enums.hpp"
#include "src/config.hpp"
#include "src/location_info.hpp"
#include "src/static_background_set.hpp"

using namespace dynamic_paper_test;
using namespace dynamic_paper;
using namespace testing;

// ===== Constants ===============

namespace {

constexpr std::string_view IMAGE_DIR = "./test_dir";
constexpr std::string_view CACHE_DIR = "./test_cache_dir";

} // namespace

// ===== Test Fixture ===============

class StaticBackgroundTest : public testing::Test {
public:
  void SetUp() override {}

  Config config = {
      std::filesystem::path(), std::nullopt, std::filesystem::path(CACHE_DIR),
      LocationInfo{.latitudeAndLongitude = {0, 0},
                   .useLatitudeAndLongitudeOverLocationSearch = false}};
  std::filesystem::path testDataDir = IMAGE_DIR;
};

// ===== Helper Functions ===============

namespace {

std::filesystem::path image(const std::string_view name) {
  return std::filesystem::path(IMAGE_DIR) / name;
}

void showBackgroundOnStaticData(StaticBackgroundTest &test,
                                TestBackgroundSetterHistory &history,
                                const std::vector<std::string> &imageNames,
                                const BackgroundSetMode mode) {
  auto setBackgroundFunc = [&history](const std::filesystem::path &imagePath,
                                      BackgroundSetMode mode) -> void {
    history.addEvent(SetEvent{imagePath, mode});
  };

  const StaticBackgroundData staticData(test.testDataDir, mode, imageNames);

  staticData.show(test.config, setBackgroundFunc);
}

} // namespace

// ===== Tests ===============

TEST_F(StaticBackgroundTest, ShowBasic) {
  TestBackgroundSetterHistory history{};
  const BackgroundSetMode mode = BackgroundSetMode::Center;

  showBackgroundOnStaticData(*this, history, {"1.jpg"}, mode);

  EXPECT_THAT(history.getHistory(),
              ElementsAre(SetEvent{.imagePath = image("1.jpg"), .mode = mode}));
}

TEST_F(StaticBackgroundTest, ShowBasicTile) {
  TestBackgroundSetterHistory history{};
  const BackgroundSetMode mode = BackgroundSetMode::Tile;

  showBackgroundOnStaticData(*this, history, {"1.jpg"}, mode);

  EXPECT_THAT(history.getHistory(),
              ElementsAre(SetEvent{.imagePath = image("1.jpg"), .mode = mode}));
}

TEST_F(StaticBackgroundTest, ShowBasicScale) {
  TestBackgroundSetterHistory history{};
  const BackgroundSetMode mode = BackgroundSetMode::Scale;

  showBackgroundOnStaticData(*this, history, {"1.jpg"}, mode);

  EXPECT_THAT(history.getHistory(),
              ElementsAre(SetEvent{.imagePath = image("1.jpg"), .mode = mode}));
}

TEST_F(StaticBackgroundTest, ShowOneOfMultiple) {
  TestBackgroundSetterHistory history{};
  const BackgroundSetMode mode = BackgroundSetMode::Center;

  showBackgroundOnStaticData(*this, history, {"1.jpg", "2.jpg", "3.jpg"}, mode);

  EXPECT_THAT(
      history.getHistory(),
      IsSubsetOf({SetEvent{.imagePath = image("1.jpg"), .mode = mode},
                  SetEvent{.imagePath = image("2.jpg"), .mode = mode},
                  SetEvent{.imagePath = image("3.jpg"), .mode = mode}}));
}
