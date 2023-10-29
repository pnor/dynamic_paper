/**
 *   Test parsing of YAML background sets
 */

#include <gtest/gtest.h>

#include <yaml-cpp/yaml.h>

#include "src/background_set.hpp"

using namespace dynamic_paper;

// Static
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
static const std::string DYNAMIC_BACKGROUND_SET_RANDOM = R""""(
dynamic_paper:
  data_directory: "./backgrounds/dynamic"
  type: dynamic
  mode: center
  transition: true
  transition_length: 55
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

// ===== Tests ====================

TEST(StaticBackgroundSetOneImage, GeneralConfig) {
  YAML::Node yaml = YAML::Load(STATIC_BACKGROUND_SET);
  auto yamlMap = yaml.as<std::unordered_map<std::string, YAML::Node>>();

  std::optional<BackgroundSet> optBackgroundSet;
  for (const auto &kv : yamlMap) {
    optBackgroundSet = std::make_optional(parseFromYAML(kv.first, kv.second));
    break;
  }

  EXPECT_TRUE(optBackgroundSet.has_value());

  const BackgroundSet &backgroundSet = optBackgroundSet.value();
  EXPECT_EQ(backgroundSet.name, "static_paper");

  const StaticBackgroundData *staticData =
      std::get_if<StaticBackgroundData>(&backgroundSet.type);
  EXPECT_EQ(staticData->dataDirectory, std::filesystem::path("~/backgrounds"));
  EXPECT_EQ(staticData->imageNames, std::vector<std::string>({"1.jpg"}));
  EXPECT_EQ(staticData->mode, BackgroundSetMode::Center);
}

TEST(StaticBackgroundSetImageList, GeneralConfig) {
  YAML::Node yaml = YAML::Load(STATIC_BACKGROUND_IMAGE_LIST_SET);
  auto yamlMap = yaml.as<std::unordered_map<std::string, YAML::Node>>();

  std::optional<BackgroundSet> optBackgroundSet;
  for (const auto &kv : yamlMap) {
    optBackgroundSet = std::make_optional(parseFromYAML(kv.first, kv.second));
    break;
  }

  EXPECT_TRUE(optBackgroundSet.has_value());

  const BackgroundSet &backgroundSet = optBackgroundSet.value();
  EXPECT_EQ(backgroundSet.name, "static_paper");

  const StaticBackgroundData *staticData =
      std::get_if<StaticBackgroundData>(&backgroundSet.type);
  EXPECT_EQ(staticData->dataDirectory, std::filesystem::path("~/backgrounds2"));
  EXPECT_EQ(staticData->imageNames,
            std::vector<std::string>({"1.jpg", "2.jpg"}));
  EXPECT_EQ(staticData->mode, BackgroundSetMode::Fill);
}

TEST(DynamicBackgroundSet, BackgroundSet) {
  // TODO
}

TEST(DynamicBackgroundSetEmptyDefaults, BackgroundSet) {
  // TODO
}

TEST(DynamicBackgroundSetSunTimes, BackgroundSet) {
  // TODO
}

TEST(DynamicBackgroundSetStrictTimes, BackgroundSet) {
  // TODO
}
