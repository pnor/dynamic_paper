/**
 *   Test parsing of YAML background sets
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <yaml-cpp/yaml.h>

#include "src/background_set.hpp"
#include "src/config.hpp"
#include "src/file_util.hpp"

#include "constants.hpp"

using namespace dynamic_paper;
using namespace dynamic_paper_test;
using namespace testing;

namespace {

Config getConfig() {
  return {"./images", BackgroundSetterMethodWallUtils(),
          SunEventPollerMethod::Dummy, std::nullopt, "~/.cache/backgrounds"};
}

// Static
const std::string_view STATIC_BACKGROUND_SET = R""""(
static_paper:
  data_directory: "~/backgrounds"
  type: static
  mode: center
  image: 1.jpg
)"""";

const std::string_view STATIC_BACKGROUND_IMAGE_LIST_SET = R""""(
static_paper:
  data_directory: "~/backgrounds2"
  type: static
  mode: fill
  images:
    - 1.jpg
    - 2.jpg
)"""";

// Dynamic
const std::string_view DYNAMIC_BACKGROUND_SET = R""""(
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

const std::string_view DYNAMIC_BACKGROUND_SET_RANDOM = R""""(
dynamic_paper:
  data_directory: "./backgrounds/dynamic"
  type: dynamic
  mode: center
  transition_length: 55
  number_transition_steps: 10
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

const std::string_view DYNAMIC_BACKGROUND_SET_STEPS_NO_LENGTH = R""""(
dynamic_paper:
  data_directory: "./backgrounds/dynamic"
  type: dynamic
  mode: center
  number_transition_steps: 10
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

const std::string_view DYNAMIC_BACKGROUND_SET_EMPTY = R""""(
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

const std::string_view DYNAMIC_BACKGROUND_SUNTIMES = R""""(
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

const std::string_view DYNAMIC_BACKGROUND_STRICT_TIMES = R""""(
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

const std::string_view DYNAMIC_BACKGROUND_NO_IMAGES_OR_TIMES = R""""(
suntimes:
  data_directory: "./backgrounds/dynamic"
  type: dynamic
  images:
  times:
)"""";

// ===== Helper ===================
BackgroundSet getBackgroundSetFrom(const std::string_view yamlString) {
  const YAML::Node yaml = YAML::Load(std::string(yamlString));
  auto yamlMap = yaml.as<std::unordered_map<std::string, YAML::Node>>();

  // yamlMap only has 1 entry; mapping the name to all the yaml info
  for (const auto &keyValue : yamlMap) {
    tl::expected<BackgroundSet, BackgroundSetParseErrors> expBackgroundSet =
        parseFromYAML(keyValue.first, keyValue.second, getConfig());
    EXPECT_TRUE(expBackgroundSet.has_value());
    return expBackgroundSet.value();
  }

  throw std::logic_error(
      "Reached impossible cast when getting background set from string");
}

} // namespace

// ===== Tests ====================

TEST(BackgroundSetTests, StaticBackgroundSetOneImage) {
  BackgroundSet backgroundSet = getBackgroundSetFrom(STATIC_BACKGROUND_SET);
  EXPECT_EQ(backgroundSet.getName(), "static_paper");

  std::optional<StaticBackgroundData> staticDataOpt =
      backgroundSet.getStaticBackgroundData();
  EXPECT_TRUE(staticDataOpt.has_value());
  assert(staticDataOpt.has_value());

  EXPECT_EQ(staticDataOpt->dataDirectory,
            getHomeDirectory() / std::filesystem::path("backgrounds"));
  EXPECT_EQ(staticDataOpt->imageNames, std::vector<std::string>({"1.jpg"}));
  EXPECT_EQ(staticDataOpt->mode, BackgroundSetMode::Center);
}

TEST(BackgroundSetTests, StaticBackgroundSetImageList) {
  BackgroundSet backgroundSet =
      getBackgroundSetFrom(STATIC_BACKGROUND_IMAGE_LIST_SET);
  EXPECT_EQ(backgroundSet.getName(), "static_paper");

  const std::optional<StaticBackgroundData> staticData =
      backgroundSet.getStaticBackgroundData();
  EXPECT_TRUE(staticData.has_value());
  assert(staticData.has_value());

  EXPECT_EQ(staticData->dataDirectory,
            getHomeDirectory() / std::filesystem::path("backgrounds2"));
  EXPECT_EQ(staticData->imageNames,
            std::vector<std::string>({"1.jpg", "2.jpg"}));
  EXPECT_EQ(staticData->mode, BackgroundSetMode::Fill);
}

TEST(BackgroundSetTests, DynamicBackgroundSet) {
  BackgroundSet backgroundSet = getBackgroundSetFrom(DYNAMIC_BACKGROUND_SET);

  EXPECT_EQ(backgroundSet.getName(), "dynamic_paper");

  const std::optional<DynamicBackgroundData> dynamicData =
      backgroundSet.getDynamicBackgroundData();
  EXPECT_TRUE(dynamicData.has_value());
  assert(dynamicData.has_value());

  EXPECT_EQ(dynamicData->dataDirectory,
            std::filesystem::path("./backgrounds/dynamic"));

  EXPECT_EQ(dynamicData->mode, BackgroundSetMode::Center);

  EXPECT_EQ(dynamicData->transition.has_value(), true);
  assert(dynamicData->transition.has_value());
  EXPECT_EQ(dynamicData->transition->duration, std::chrono::seconds(55));

  EXPECT_EQ(dynamicData->order, BackgroundSetOrder::Linear);

  ASSERT_THAT(dynamicData->imageNames,
              ElementsAre("1.jpg", "2.jpg", "3.jpg", "4.jpg"));

  ASSERT_THAT(dynamicData->times,
              ElementsAre(DUMMY_SUNRISE_TIME - ONE_HOUR, DUMMY_SUNRISE_TIME,
                          DUMMY_SUNSET_TIME - ONE_HOUR, DUMMY_SUNSET_TIME));
}

TEST(BackgroundSetTests, DynamicBackgroundSetRandom) {
  BackgroundSet backgroundSet =
      getBackgroundSetFrom(DYNAMIC_BACKGROUND_SET_RANDOM);

  EXPECT_EQ(backgroundSet.getName(), "dynamic_paper");

  const std::optional<DynamicBackgroundData> dynamicData =
      backgroundSet.getDynamicBackgroundData();
  EXPECT_TRUE(dynamicData.has_value());
  assert(dynamicData.has_value());

  EXPECT_EQ(dynamicData->dataDirectory,
            std::filesystem::path("./backgrounds/dynamic"));

  EXPECT_EQ(dynamicData->mode, BackgroundSetMode::Center);

  EXPECT_TRUE(dynamicData->transition.has_value());
  assert(dynamicData->transition.has_value());
  EXPECT_EQ(dynamicData->transition->duration, std::chrono::seconds(55));
  EXPECT_EQ(dynamicData->transition->steps, 10);

  EXPECT_EQ(dynamicData->order, BackgroundSetOrder::Random);

  ASSERT_THAT(dynamicData->imageNames, ElementsAre("1.jpg", "2.jpg", "3.jpg"));

  ASSERT_THAT(dynamicData->times,
              ElementsAre(ZERO_TIME, ZERO_TIME + (4 * ONE_HOUR),
                          ZERO_TIME + (10 * ONE_HOUR),
                          ZERO_TIME + (16 * ONE_HOUR)));
}

TEST(BackgroundSetTests, DynamicBackgroundSetEmptyDefaults) {
  BackgroundSet backgroundSet =
      getBackgroundSetFrom(DYNAMIC_BACKGROUND_SET_EMPTY);

  EXPECT_EQ(backgroundSet.getName(), "dynamic_paper");

  const std::optional<DynamicBackgroundData> dynamicData =
      backgroundSet.getDynamicBackgroundData();
  EXPECT_TRUE(dynamicData.has_value());
  assert(dynamicData.has_value());

  EXPECT_EQ(dynamicData->dataDirectory,
            std::filesystem::path("./backgrounds/dynamic"));

  EXPECT_EQ(dynamicData->mode, BackgroundSetMode::Center);

  EXPECT_EQ(dynamicData->transition, std::nullopt);

  EXPECT_EQ(dynamicData->order, BackgroundSetOrder::Linear);

  ASSERT_THAT(dynamicData->imageNames, ElementsAre("1.jpg", "2.jpg"));

  ASSERT_THAT(dynamicData->times, ElementsAre(ZERO_TIME, ZERO_TIME + ONE_HOUR));
}

TEST(BackgroundSetTests, DynamicBackgroundSetSunTimes) {
  BackgroundSet backgroundSet =
      getBackgroundSetFrom(DYNAMIC_BACKGROUND_SUNTIMES);

  EXPECT_EQ(backgroundSet.getName(), "suntimes");

  const std::optional<DynamicBackgroundData> dynamicData =
      backgroundSet.getDynamicBackgroundData();
  EXPECT_TRUE(dynamicData.has_value());
  assert(dynamicData.has_value());

  EXPECT_EQ(dynamicData->dataDirectory,
            std::filesystem::path("./backgrounds/dynamic"));

  EXPECT_EQ(dynamicData->mode, BackgroundSetMode::Center);

  EXPECT_EQ(dynamicData->transition, std::nullopt);

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
  BackgroundSet backgroundSet =
      getBackgroundSetFrom(DYNAMIC_BACKGROUND_STRICT_TIMES);

  EXPECT_EQ(backgroundSet.getName(), "suntimes");

  const std::optional<DynamicBackgroundData> dynamicData =
      backgroundSet.getDynamicBackgroundData();
  EXPECT_TRUE(dynamicData.has_value());
  assert(dynamicData.has_value());

  EXPECT_EQ(dynamicData->dataDirectory,
            std::filesystem::path("./backgrounds/dynamic"));

  EXPECT_EQ(dynamicData->mode, BackgroundSetMode::Center);

  EXPECT_EQ(dynamicData->transition, std::nullopt);

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
  const YAML::Node yaml =
      YAML::Load(std::string(DYNAMIC_BACKGROUND_NO_IMAGES_OR_TIMES));
  auto yamlMap = yaml.as<std::unordered_map<std::string, YAML::Node>>();

  for (const auto &keyValue : yamlMap) {
    tl::expected<BackgroundSet, BackgroundSetParseErrors> expBackgroundSet =
        parseFromYAML(keyValue.first, keyValue.second, getConfig());
    EXPECT_FALSE(expBackgroundSet.has_value());
    EXPECT_EQ(expBackgroundSet.error(), BackgroundSetParseErrors::NoImages);
  }
}

TEST(BackgroundSetTests, DynamicBackgroundSetTransitionStepsNoLength) {
  BackgroundSet backgroundSet =
      getBackgroundSetFrom(DYNAMIC_BACKGROUND_SET_STEPS_NO_LENGTH);

  EXPECT_EQ(backgroundSet.getName(), "dynamic_paper");

  const std::optional<DynamicBackgroundData> dynamicData =
      backgroundSet.getDynamicBackgroundData();
  EXPECT_TRUE(dynamicData.has_value());
  assert(dynamicData.has_value());

  EXPECT_EQ(dynamicData->transition, std::nullopt);
}
