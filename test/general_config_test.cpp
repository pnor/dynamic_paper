/**
 *   Test parsing of YAML config files
 */
#include <cmath>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "src/config.hpp"
#include "src/defaults.hpp"
#include "src/file_util.hpp"

using namespace dynamic_paper;

namespace {

constexpr std::string_view GENERAL_CONFIG_YAML = R""""(
sun_poller: "sunwait"
background_config: "./an_image_dir"
hook_script: "./hook_script.sh"
cache_dir: "~/.cache/backgrounds"
)"""";

constexpr std::string EMPTY_YAML;

constexpr std::string_view LATITUDE_AND_LONGITUDE = R""""(
background_config: "./image"
latitude: 70.0
longitude: 70.0
)"""";

constexpr std::string_view ONLY_LATITUDE = R""""(
background_config: "./image"
latitude: 70.0
)"""";

constexpr std::string_view ONLY_LONGITUDE = R""""(
background_config: "./image"
longitude: 70.0
)"""";

constexpr std::string_view NO_LATITUDE_OR_LONGITUDE = R""""(
background_config: "./image"
)"""";

constexpr std::string_view GET_LATITUDE_LONGITUDE_FROM_FILE = R""""(
background_config: "./image"
latitude: 70.0
longitude: 70.0
use_config_file_location: true
)"""";

constexpr std::string_view GET_LATITUDE_LONGITUDE_FROM_FILE_NO_LATLONG = R""""(
background_config: "./image"
use_config_file_location: true
)"""";

constexpr std::string_view GET_LATITUDE_LONGITUDE_FROM_FILE_JUST_LATITUDE =
    R""""(
background_config: "./image"
use_config_file_location: true
latitude: 70.0
)"""";

tl::expected<Config, ConfigError>
loadConfigFromString(const std::string_view configString) {
  return loadConfigFromYAML(YAML::Load(std::string(configString)));
}

inline bool doublesCloseTo(const std::pair<double, double> &actual,
                           const std::pair<double, double> &expected) {
  return (std::abs(actual.first - expected.first) <
          std::numeric_limits<double>::epsilon()) &&
         (std::abs(actual.second - expected.second) <
          std::numeric_limits<double>::epsilon());
}

inline bool locationInfoSameAsDefault(const LocationInfo &locationInfo) {
  const LocationInfo defaultInfo = ConfigDefaults::locationInfo;
  return doublesCloseTo(locationInfo.latitudeAndLongitude,
                        defaultInfo.latitudeAndLongitude) &&
         (locationInfo.useLocationInfoOverSearch ==
          defaultInfo.useLocationInfoOverSearch);
}

} // namespace

TEST(GeneralConfig, AllFilled) {
  const tl::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(GENERAL_CONFIG_YAML);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &cfg = expectedConfig.value();

  EXPECT_EQ(cfg.sunEventPollerMethod, SunEventPollerMethod::Sunwait);
  EXPECT_EQ(cfg.backgroundSetConfigFile,
            std::filesystem::path("./an_image_dir"));
  EXPECT_EQ(cfg.hookScript,
            std::make_optional(std::filesystem::path("./hook_script.sh")));
  EXPECT_EQ(cfg.imageCacheDirectory,
            getHomeDirectory() / std::filesystem::path(".cache/backgrounds"));
}

TEST(GeneralConfig, DefaultValues) {
  const tl::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(EMPTY_YAML);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &cfg = expectedConfig.value();

  EXPECT_EQ(cfg.sunEventPollerMethod, SunEventPollerMethod::Sunwait);
  EXPECT_EQ(cfg.backgroundSetConfigFile,
            std::filesystem::path(ConfigDefaults::backgroundSetConfigFile()));
  EXPECT_EQ(cfg.hookScript, std::nullopt);
  EXPECT_EQ(cfg.imageCacheDirectory,
            std::filesystem::path(ConfigDefaults::imageCacheDirectory()));
}

TEST(GeneralConfig, LatLongBothProvided) {
  const tl::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(LATITUDE_AND_LONGITUDE);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &config = expectedConfig.value();

  EXPECT_TRUE(doublesCloseTo(config.locationInfo.latitudeAndLongitude,
                             std::make_pair(70.0, 70.0)));
  EXPECT_FALSE(config.locationInfo.useLocationInfoOverSearch);
}

TEST(GeneralConfig, LatLongOnlyLatitude) {
  const tl::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(ONLY_LATITUDE);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &config = expectedConfig.value();
  EXPECT_TRUE(locationInfoSameAsDefault(config.locationInfo));
}

TEST(GeneralConfig, LatLongOnlyLongitude) {
  const tl::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(ONLY_LONGITUDE);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &config = expectedConfig.value();
  EXPECT_TRUE(locationInfoSameAsDefault(config.locationInfo));
}
TEST(GeneralConfig, LatLongNoLatitudeOrLongitude) {
  const tl::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(NO_LATITUDE_OR_LONGITUDE);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &config = expectedConfig.value();
  EXPECT_TRUE(locationInfoSameAsDefault(config.locationInfo));
}

TEST(GeneralConfig, LatLongGetFromFile) {
  const tl::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(GET_LATITUDE_LONGITUDE_FROM_FILE);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &config = expectedConfig.value();

  EXPECT_TRUE(doublesCloseTo(config.locationInfo.latitudeAndLongitude,
                             std::make_pair(70.0, 70.0)));
  EXPECT_TRUE(config.locationInfo.useLocationInfoOverSearch);
}

TEST(GeneralConfig, LatLongGetFromFileNoLatLong) {
  const tl::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(GET_LATITUDE_LONGITUDE_FROM_FILE_NO_LATLONG);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &config = expectedConfig.value();
  EXPECT_TRUE(locationInfoSameAsDefault(config.locationInfo));
}

TEST(GeneralConfig, LatLongGetFromFileJustLatitude) {
  const tl::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(GET_LATITUDE_LONGITUDE_FROM_FILE_JUST_LATITUDE);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &config = expectedConfig.value();
  EXPECT_TRUE(locationInfoSameAsDefault(config.locationInfo));
}
