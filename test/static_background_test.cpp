/**
 * Test ability to show Static Backgrounds Sets
 */

#include <filesystem>
#include <string_view>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <tl/expected.hpp>

#include "background_test_setter.hpp"
#include "src/background_set_enums.hpp"
#include "src/config.hpp"
#include "src/static_background_set.hpp"

using namespace dynamic_paper_test;
using namespace dynamic_paper;
using namespace testing;

// ===== Constants ===============

namespace {

constexpr std::string_view DATA_DIR = "./test_dir";
constexpr std::string_view CACHE_DIR = "./test_cache_dir";

} // namespace

// ===== Test Fixture ===============

class StaticBackgroundTest : public testing::Test {
public:
  void SetUp() override {}

  Config config = {std::filesystem::path(),
                   {BackgroundSetterMethodWallUtils()},
                   SunEventPollerMethod::Dummy,
                   std::nullopt,
                   std::filesystem::path(CACHE_DIR)};
  std::filesystem::path testDataDir = DATA_DIR;
};

// ===== Helper Functions ===============

namespace {

std::filesystem::path image(const std::string_view name) {
  return std::filesystem::path(DATA_DIR) / name;
}

void showBackgroundOnStaticData(StaticBackgroundTest &test,
                                TestBackgroundSetterHistory &history,
                                const std::vector<std::string> &imageNames,
                                const BackgroundSetMode mode) {
  auto setBackgroundFunc = [&history](const std::filesystem::path &imagePath,
                                      BackgroundSetMode mode,
                                      const BackgroundSetterMethod &)
      -> tl::expected<void, BackgroundError> {
    history.addEvent(SetEvent{imagePath, mode});
    return {};
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
