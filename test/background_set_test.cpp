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
static const Config config("./images", BackgroundSetterMethodWallUtils(),
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

static const std::string DYNAMIC_BACKGROUND_SUNTIMES = R""""(
suntimes:
  data_directory: "./backgrounds/dynamic"
  type: dynamic
  images:
    -  1.jpg
    -  2.jpg
    -  3.jpg
    -  4.jpg
    -  5.jpg
    -  6.jpg
    -  7.jpg
    -  8.jpg
    -  9.jpg
    - 10.jpg
    - 11.jpg
    - 12.jpg
  times:
    - "-5:00 sunrise"
    - "-04:00 sunrise"
    - "-03:30 sunrise"
    - "-1:11 sunrise"
    - "sunrise"
    - "+0:00 sunrise"
    - "+00:00 sunrise"
    - "+1:00 sunrise"
    - "+01:01 sunrise"
    - "-1:00 sunset"
    - "-00:00 sunset"
    - "sunset"
)"""";

static const std::string DYNAMIC_BACKGROUND_STRICT_TIMES = R""""(
suntimes:
  data_directory: "./backgrounds/dynamic"
  type: dynamic
  images:
    -  1.jpg
    -  2.jpg
    -  3.jpg
    -  4.jpg
    -  5.jpg
    -  6.jpg
    -  7.jpg
    -  8.jpg
  times:
    - "0:00"
    - "00:00"
    - "1:30"
    - "02:45"
    - "11:11"
    - "15:59"
    - "23:23"
    - "23:59"
)"""";

static const std::string DYNAMIC_BACKGROUND_NO_IMAGES_OR_TIMES = R""""(
suntimes:
  data_directory: "./backgrounds/dynamic"
  type: dynamic
  images:
  times:
)"""";

// ===== Helper ===================
static BackgroundSet getBackgroundSetFrom(const std::string &s,
                                          const Config &config) {
  YAML::Node yaml = YAML::Load(s);
  auto yamlMap = yaml.as<std::unordered_map<std::string, YAML::Node>>();

  // yamlMap only has 1 entry; mapping the name to all the yaml info
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
  const BackgroundSet backgroundSet =
      getBackgroundSetFrom(DYNAMIC_BACKGROUND_SUNTIMES, config);

  EXPECT_EQ(backgroundSet.name, "suntimes");

  const DynamicBackgroundData *dynamicData =
      std::get_if<DynamicBackgroundData>(&backgroundSet.type);

  EXPECT_EQ(dynamicData->dataDirectory,
            std::filesystem::path("./backgrounds/dynamic"));

  EXPECT_EQ(dynamicData->mode, BackgroundSetMode::Center);

  EXPECT_EQ(dynamicData->transitionDuration, std::nullopt);

  EXPECT_EQ(dynamicData->order, BackgroundSetOrder::Linear);

  ASSERT_THAT(dynamicData->imageNames,
              ElementsAre("1.jpg", "2.jpg", "3.jpg", "4.jpg", "5.jpg", "6.jpg",
                          "7.jpg", "8.jpg", "9.jpg", "10.jpg", "11.jpg",
                          "12.jpg"));

  ASSERT_THAT(
      dynamicData->times,
      ElementsAre(DUMMY_SUNRISE_TIME - ((5 * ONE_HOUR) + (0 * ONE_MINUTE)),
                  DUMMY_SUNRISE_TIME - ((4 * ONE_HOUR) + (0 * ONE_MINUTE)),
                  DUMMY_SUNRISE_TIME - ((3 * ONE_HOUR) + (30 * ONE_MINUTE)),
                  DUMMY_SUNRISE_TIME - ((1 * ONE_HOUR) + (11 * ONE_MINUTE)),
                  DUMMY_SUNRISE_TIME,
                  DUMMY_SUNRISE_TIME + ((0 * ONE_HOUR) + (0 * ONE_MINUTE)),
                  DUMMY_SUNRISE_TIME + ((0 * ONE_HOUR) + (0 * ONE_MINUTE)),
                  DUMMY_SUNRISE_TIME + ((1 * ONE_HOUR) + (0 * ONE_MINUTE)),
                  DUMMY_SUNRISE_TIME + ((1 * ONE_HOUR) + (1 * ONE_MINUTE)),
                  DUMMY_SUNSET_TIME - ((1 * ONE_HOUR) + (0 * ONE_MINUTE)),
                  DUMMY_SUNSET_TIME - ((0 * ONE_HOUR) + (0 * ONE_MINUTE)),
                  DUMMY_SUNSET_TIME));
}

TEST(BackgroundSetTests, DynamicBackgroundSetStrictTimes) {
  const BackgroundSet backgroundSet =
      getBackgroundSetFrom(DYNAMIC_BACKGROUND_STRICT_TIMES, config);

  EXPECT_EQ(backgroundSet.name, "suntimes");

  const DynamicBackgroundData *dynamicData =
      std::get_if<DynamicBackgroundData>(&backgroundSet.type);

  EXPECT_EQ(dynamicData->dataDirectory,
            std::filesystem::path("./backgrounds/dynamic"));

  EXPECT_EQ(dynamicData->mode, BackgroundSetMode::Center);

  EXPECT_EQ(dynamicData->transitionDuration, std::nullopt);

  EXPECT_EQ(dynamicData->order, BackgroundSetOrder::Linear);

  ASSERT_THAT(dynamicData->imageNames,
              ElementsAre("1.jpg", "2.jpg", "3.jpg", "4.jpg", "5.jpg", "6.jpg",
                          "7.jpg", "8.jpg"));

  ASSERT_THAT(dynamicData->times,
              ElementsAre(ZERO_TIME + ((0 * ONE_HOUR) + (0 * ONE_MINUTE)),
                          ZERO_TIME + ((0 * ONE_HOUR) + (0 * ONE_MINUTE)),
                          ZERO_TIME + ((1 * ONE_HOUR) + (30 * ONE_MINUTE)),
                          ZERO_TIME + ((2 * ONE_HOUR) + (45 * ONE_MINUTE)),
                          ZERO_TIME + ((11 * ONE_HOUR) + (11 * ONE_MINUTE)),
                          ZERO_TIME + ((15 * ONE_HOUR) + (59 * ONE_MINUTE)),
                          ZERO_TIME + ((23 * ONE_HOUR) + (23 * ONE_MINUTE)),
                          ZERO_TIME + ((23 * ONE_HOUR) + (59 * ONE_MINUTE))));
}

TEST(BackgroundSetTests, DynamicBackgroundSetNoImagesOrTimes) {
  YAML::Node yaml = YAML::Load(DYNAMIC_BACKGROUND_NO_IMAGES_OR_TIMES);
  auto yamlMap = yaml.as<std::unordered_map<std::string, YAML::Node>>();

  for (const auto &kv : yamlMap) {
    std::expected<BackgroundSet, BackgroundSetParseErrors> expBackgroundSet =
        parseFromYAML(kv.first, kv.second, config);
    EXPECT_FALSE(expBackgroundSet.has_value());
    EXPECT_EQ(expBackgroundSet.error(), BackgroundSetParseErrors::NoImages);
  }
}
