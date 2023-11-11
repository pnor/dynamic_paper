/**
 *   Test parsing of YAML background sets
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <yaml-cpp/yaml.h>

#include "src/background_set.hpp"
#include "src/config.hpp"

#include "constants.hpp"

using namespace dynamic_paper;
using namespace dynamic_paper_test;
using namespace testing;

// Static
static const Config config("./images", BackgroundSetterMethod::WallUtils,
                           SunEventPollerMethod::Dummy, std::nullopt);

static const std::string STATIC_BACKGROUND_SET = R""""(
static_paper:
  data_directory: "~/backgrounds"
  type: static
  mode: center
  image: 1.jpg
)"""";

static const std::string STATIC_BACKGROUND_IMAGE_LIST_SET = R""""(
static_paper:
  data_directory: "~/backgrounds2"
  type: static
  mode: fill
  images:
    - 1.jpg
    - 2.jpg
)"""";

// Dynamic
static const std::string DYNAMIC_BACKGROUND_SET = R""""(
dynamic_paper:
  data_directory: "./backgrounds/dynamic"
  type: dynamic
  mode: center
  transition_length: 55
  order: linear
  images:
    - 1.jpg
    - 2.jpg
    - 3.jpg
    - 4.jpg
  times:
    - "-1:00 sunrise"
    - "sunrise"
    - "-1:00 sunset"
    - "sunset"
)"""";

static const std::string DYNAMIC_BACKGROUND_SET_RANDOM = R""""(
dynamic_paper:
  data_directory: "./backgrounds/dynamic"
  type: dynamic
  mode: center
  transition_length: 55
  order: random
  images:
    - 1.jpg
    - 2.jpg
    - 3.jpg
  times:
    - "00:00"
    - "04:00"
    - "10:00"
    - "16:00"
)"""";

static const std::string DYNAMIC_BACKGROUND_SET_EMPTY = R""""(
dynamic_paper:
  data_directory: "./backgrounds/dynamic"
  type: dynamic
  images:
    - 1.jpg
    - 2.jpg
  times:
    - "00:00"
    - "01:00"
)"""";

// ===== Helper ===================
static BackgroundSet getBackgroundSetFrom(const std::string &s,
                                          const Config &config) {
  YAML::Node yaml = YAML::Load(s);
  auto yamlMap = yaml.as<std::unordered_map<std::string, YAML::Node>>();

  for (const auto &kv : yamlMap) {
    std::expected<BackgroundSet, BackgroundSetParseErrors> expBackgroundSet =
        parseFromYAML(kv.first, kv.second, config);
    EXPECT_TRUE(expBackgroundSet.has_value());
    return expBackgroundSet.value();
  }

  throw std::logic_error(
      "Reached impossible cast when getting background set from string");
}

// ===== Tests ====================

TEST(BackgroundSetTests, StaticBackgroundSetOneImage) {
  const BackgroundSet backgroundSet =
      getBackgroundSetFrom(STATIC_BACKGROUND_SET, config);
  EXPECT_EQ(backgroundSet.name, "static_paper");

  const StaticBackgroundData *staticData =
      std::get_if<StaticBackgroundData>(&backgroundSet.type);
  EXPECT_EQ(staticData->dataDirectory, std::filesystem::path("~/backgrounds"));
  EXPECT_EQ(staticData->imageNames, std::vector<std::string>({"1.jpg"}));
  EXPECT_EQ(staticData->mode, BackgroundSetMode::Center);
}

TEST(BackgroundSetTests, StaticBackgroundSetImageList) {
  const BackgroundSet backgroundSet =
      getBackgroundSetFrom(STATIC_BACKGROUND_IMAGE_LIST_SET, config);
  EXPECT_EQ(backgroundSet.name, "static_paper");

  const StaticBackgroundData *staticData =
      std::get_if<StaticBackgroundData>(&backgroundSet.type);
  EXPECT_EQ(staticData->dataDirectory, std::filesystem::path("~/backgrounds2"));
  EXPECT_EQ(staticData->imageNames,
            std::vector<std::string>({"1.jpg", "2.jpg"}));
  EXPECT_EQ(staticData->mode, BackgroundSetMode::Fill);
}

TEST(BackgroundSetTests, DynamicBackgroundSet) {
  const BackgroundSet backgroundSet =
      getBackgroundSetFrom(DYNAMIC_BACKGROUND_SET, config);

  EXPECT_EQ(backgroundSet.name, "dynamic_paper");

  const DynamicBackgroundData *dynamicData =
      std::get_if<DynamicBackgroundData>(&backgroundSet.type);

  EXPECT_EQ(dynamicData->dataDirectory,
            std::filesystem::path("./backgrounds/dynamic"));

  EXPECT_EQ(dynamicData->mode, BackgroundSetMode::Center);

  EXPECT_EQ(dynamicData->transitionDuration, std::make_optional(55));

  EXPECT_EQ(dynamicData->order, BackgroundSetOrder::Linear);

  ASSERT_THAT(dynamicData->imageNames,
              ElementsAre("1.jpg", "2.jpg", "3.jpg", "4.jpg"));

  ASSERT_THAT(dynamicData->times,
              ElementsAre(DUMMY_SUNRISE_TIME - ONE_HOUR, DUMMY_SUNRISE_TIME,
                          DUMMY_SUNSET_TIME - ONE_HOUR, DUMMY_SUNSET_TIME));
}

TEST(BackgroundSetTests, DynamicBackgroundSetRandom) {
  const BackgroundSet backgroundSet =
      getBackgroundSetFrom(DYNAMIC_BACKGROUND_SET_RANDOM, config);

  EXPECT_EQ(backgroundSet.name, "dynamic_paper");

  const DynamicBackgroundData *dynamicData =
      std::get_if<DynamicBackgroundData>(&backgroundSet.type);

  EXPECT_EQ(dynamicData->dataDirectory,
            std::filesystem::path("./backgrounds/dynamic"));

  EXPECT_EQ(dynamicData->mode, BackgroundSetMode::Center);

  EXPECT_EQ(dynamicData->transitionDuration, std::make_optional(55));

  EXPECT_EQ(dynamicData->order, BackgroundSetOrder::Random);

  ASSERT_THAT(dynamicData->imageNames, ElementsAre("1.jpg", "2.jpg", "3.jpg"));

  ASSERT_THAT(dynamicData->times,
              ElementsAre(ZERO_TIME, ZERO_TIME + (4 * ONE_HOUR),
                          ZERO_TIME + (10 * ONE_HOUR),
                          ZERO_TIME + (16 * ONE_HOUR)));
}

TEST(BackgroundSetTests, DynamicBackgroundSetEmptyDefaults) {
  const BackgroundSet backgroundSet =
      getBackgroundSetFrom(DYNAMIC_BACKGROUND_SET_EMPTY, config);

  EXPECT_EQ(backgroundSet.name, "dynamic_paper");

  const DynamicBackgroundData *dynamicData =
      std::get_if<DynamicBackgroundData>(&backgroundSet.type);

  EXPECT_EQ(dynamicData->dataDirectory,
            std::filesystem::path("./backgrounds/dynamic"));

  EXPECT_EQ(dynamicData->mode, BackgroundSetMode::Center);

  EXPECT_EQ(dynamicData->transitionDuration, std::nullopt);

  EXPECT_EQ(dynamicData->order, BackgroundSetOrder::Linear);

  ASSERT_THAT(dynamicData->imageNames, ElementsAre("1.jpg", "2.jpg"));

  ASSERT_THAT(dynamicData->times, ElementsAre(ZERO_TIME, ZERO_TIME + ONE_HOUR));
}

TEST(BackgroundSetTests, DynamicBackgroundSetSunTimes) {
  // TODO
}

TEST(BackgroundSetTests, DynamicBackgroundSetStrictTimes) {
  // TODO
}
