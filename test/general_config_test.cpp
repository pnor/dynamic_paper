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

// TODO what to test
// test it prefers soalr over location
// both provided
// none provided
// opne provided and rthw orhwe in seperate tests
// one provided and not the other but one partially provided both tests
// both partially provided
// TODO need to mock the act of globally getting the location from mozilla
// - could manually just set it actually

// These should match the values in the strings
const double TEST_LATITUDE = 70.0;
const double TEST_LONGITUDE = 70.0;

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

inline bool solarDayMatchesSolarDayForLocation(const SolarDay solarDay,
                                               const LocationInfo location) {
  const SolarDay locationSolarDay = getSolarDayUsingLocation(location);
  return solarDay == locationSolarDay;
}

inline bool solarDayIsDefault(const SolarDay solarDay) {
  return solarDay == ConfigDefaults::solarDay;
}

} // namespace

TEST(GeneralConfig, AllFilled) {
  const tl::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(GENERAL_CONFIG_YAML);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &cfg = expectedConfig.value();

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

  EXPECT_EQ(cfg.backgroundSetConfigFile,
            std::filesystem::path(ConfigDefaults::backgroundSetConfigFile()));
  EXPECT_EQ(cfg.hookScript, std::nullopt);
  EXPECT_EQ(cfg.imageCacheDirectory,
            std::filesystem::path(ConfigDefaults::imageCacheDirectory()));
}

// Should use default solar day times if no info related to it is provided
TEST(GeneralConfig, PreferDefaultSolarDayWhenNoOptionsProvided) {
  const tl::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(EMPTY_YAML);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &cfg = expectedConfig.value();

  EXPECT_TRUE(solarDayIsDefault(cfg.solarDayProvider.getSolarDay()));
}

// Should create sunrise/sunset times using location info provided
TEST(GeneralConfig, LatLongBothProvided) {
  const tl::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(LATITUDE_AND_LONGITUDE);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &config = expectedConfig.value();

  const LocationInfo testLocation = {
      .latitudeAndLongitude = std::make_pair(TEST_LATITUDE, TEST_LONGITUDE),
      .useLatitudeAndLongitudeOverLocationSearch = false};

  EXPECT_TRUE(solarDayMatchesSolarDayForLocation(
      config.solarDayProvider.getSolarDay(), testLocation));
}

// If only location info is incomplete, treat same as having no information and
// choose the default sunrise/sunset times
TEST(GeneralConfig, LatLongOnlyLatitude) {
  const tl::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(ONLY_LATITUDE);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &config = expectedConfig.value();
  EXPECT_TRUE(solarDayIsDefault(config.solarDayProvider.getSolarDay()));
}

// If only location info is incomplete, treat same as having no information and
// choose the default sunrise/sunset times
TEST(GeneralConfig, LatLongOnlyLongitude) {
  const tl::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(ONLY_LONGITUDE);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &config = expectedConfig.value();
  EXPECT_TRUE(solarDayIsDefault(config.solarDayProvider.getSolarDay()));
}

// If only location info is incomplete, treat same as having no information and
// choose the default sunrise/sunset times
TEST(GeneralConfig, LatLongNoLatitudeOrLongitude) {
  const tl::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(NO_LATITUDE_OR_LONGITUDE);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &config = expectedConfig.value();
  EXPECT_TRUE(solarDayIsDefault(config.solarDayProvider.getSolarDay()));
}

// Getting from file for location should count as a complete location, so use
// solar day lcoation based off of it
TEST(GeneralConfig, LatLongGetFromFile) {
  const tl::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(GET_LATITUDE_LONGITUDE_FROM_FILE);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &config = expectedConfig.value();

  const LocationInfo testLocation = {
      .latitudeAndLongitude = std::make_pair(TEST_LATITUDE, TEST_LONGITUDE),
      .useLatitudeAndLongitudeOverLocationSearch = true};

  EXPECT_TRUE(solarDayMatchesSolarDayForLocation(
      config.solarDayProvider.getSolarDay(), testLocation));
}

TEST(GeneralConfig, LatLongGetFromFileNoLatLong) {
  const tl::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(GET_LATITUDE_LONGITUDE_FROM_FILE_NO_LATLONG);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &config = expectedConfig.value();
  EXPECT_TRUE(solarDayIsDefault(config.solarDayProvider.getSolarDay()));
}

TEST(GeneralConfig, LatLongGetFromFileJustLatitude) {
  const tl::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(GET_LATITUDE_LONGITUDE_FROM_FILE_JUST_LATITUDE);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &config = expectedConfig.value();
  EXPECT_TRUE(solarDayIsDefault(config.solarDayProvider.getSolarDay()));
}
