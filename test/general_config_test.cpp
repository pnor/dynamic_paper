/**
 *   Test parsing of YAML config files
 */
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <gtest/gtest.h>
#include <tl/expected.hpp>

#include "src/config.hpp"
#include "src/defaults.hpp"
#include "src/file_util.hpp"
#include "src/location_info.hpp"
#include "src/solar_day.hpp"
#include "src/time_from_midnight.hpp"

using namespace dynamic_paper;

namespace {

constexpr std::string_view GENERAL_CONFIG_YAML = R""""(
sun_poller: "sunwait"
background_config: "./an_image_dir"
hook_script: "./hook_script.sh"
cache_dir: "~/.cache/backgrounds"
)"""";

constexpr std::string EMPTY_YAML;

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

constexpr std::string_view SOLAR_DAY_AND_LOCATION_PROVIDED = R""""(
background_config: "./image"
latitude: 70.0
longitude: 70.0
use_config_file_location: true
sunrise: 10:00
sunset: 20:00
)"""";

constexpr std::string_view SOLAR_DAY_ONLY = R""""(
background_config: "./image"
sunrise: 10:00
sunset: 20:00
)"""";

constexpr std::string_view SOLAR_DAY_JUST_SUNRISE = R""""(
background_config: "./image"
sunrise: 10:00
)"""";

constexpr std::string_view SOLAR_DAY_JUST_SUNSET = R""""(
background_config: "./image"
sunset: 20:00
)"""";

constexpr std::string_view INCOMPLETE_SOLAR_DAY_AND_COMPLETE_LOCATION = R""""(
background_config: "./image"
sunset: 20:00
latitude: 70.0
longitude: 70.0
use_config_file_location: true
)"""";

Config loadConfigFromString(const std::string_view configString) {
  return loadConfigFromYAML(YAML::Load(std::string(configString)), true);
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
  const Config config = loadConfigFromString(GENERAL_CONFIG_YAML);

  EXPECT_EQ(config.backgroundSetConfigFile,
            std::filesystem::path("./an_image_dir"));
  EXPECT_EQ(config.hookScript,
            std::make_optional(std::filesystem::path("./hook_script.sh")));
  EXPECT_EQ(config.imageCacheDirectory,
            getHomeDirectory() / std::filesystem::path(".cache/backgrounds"));
}

TEST(GeneralConfig, DefaultValues) {
  const Config config = loadConfigFromString(EMPTY_YAML);

  EXPECT_EQ(config.backgroundSetConfigFile,
            std::filesystem::path(ConfigDefaults::backgroundSetConfigFile()));
  EXPECT_EQ(config.hookScript, std::nullopt);
  EXPECT_EQ(config.imageCacheDirectory,
            std::filesystem::path(ConfigDefaults::imageCacheDirectory()));
}

// Should use default solar day times if no info related to it is provided
TEST(GeneralConfig, PreferDefaultSolarDayWhenNoOptionsProvided) {
  const Config config = loadConfigFromString(EMPTY_YAML);

  EXPECT_TRUE(solarDayIsDefault(config.solarDayProvider.getSolarDay()));
}

// Should create sunrise/sunset times using location info provided
TEST(GeneralConfig, LatLongBothProvided) {
  const Config config = loadConfigFromString(LATITUDE_AND_LONGITUDE);

  const LocationInfo testLocation = {
      .latitudeAndLongitude = std::make_pair(TEST_LATITUDE, TEST_LONGITUDE),
      .useLatitudeAndLongitudeOverLocationSearch = false};

  EXPECT_TRUE(solarDayMatchesSolarDayForLocation(
      config.solarDayProvider.getSolarDay(), testLocation));
}

// If only location info is incomplete, treat same as having no information and
// choose the default sunrise/sunset times
TEST(GeneralConfig, LatLongOnlyLatitude) {
  const Config config = loadConfigFromString(ONLY_LATITUDE);
  EXPECT_TRUE(solarDayIsDefault(config.solarDayProvider.getSolarDay()));
}

// If only location info is incomplete, treat same as having no information and
// choose the default sunrise/sunset times
TEST(GeneralConfig, LatLongOnlyLongitude) {
  const Config config = loadConfigFromString(ONLY_LONGITUDE);
  EXPECT_TRUE(solarDayIsDefault(config.solarDayProvider.getSolarDay()));
}

// If only location info is incomplete, treat same as having no information and
// choose the default sunrise/sunset times
TEST(GeneralConfig, LatLongNoLatitudeOrLongitude) {
  const Config config = loadConfigFromString(NO_LATITUDE_OR_LONGITUDE);
  EXPECT_TRUE(solarDayIsDefault(config.solarDayProvider.getSolarDay()));
}

// Getting from file for location should count as a complete location, so use
// solar day lcoation based off of it
TEST(GeneralConfig, LatLongGetFromFile) {
  const Config config = loadConfigFromString(GET_LATITUDE_LONGITUDE_FROM_FILE);

  const LocationInfo testLocation = {
      .latitudeAndLongitude = std::make_pair(TEST_LATITUDE, TEST_LONGITUDE),
      .useLatitudeAndLongitudeOverLocationSearch = true};

  EXPECT_TRUE(solarDayMatchesSolarDayForLocation(
      config.solarDayProvider.getSolarDay(), testLocation));
}

// If attempts to get location but there is none provided from file, and there
// is no sunrise/sunset, should use default
TEST(GeneralConfig, LatLongGetFromFileNoLatLong) {
  const Config config =
      loadConfigFromString(GET_LATITUDE_LONGITUDE_FROM_FILE_NO_LATLONG);
  EXPECT_TRUE(solarDayIsDefault(config.solarDayProvider.getSolarDay()));
}

// If unable to get an entire location, and no sunrise/sunset is provided,
// should use default
TEST(GeneralConfig, LatLongGetFromFileJustLatitude) {
  const Config config =
      loadConfigFromString(GET_LATITUDE_LONGITUDE_FROM_FILE_JUST_LATITUDE);
  EXPECT_TRUE(solarDayIsDefault(config.solarDayProvider.getSolarDay()));
}

// If both are provided, should prefer the solar day
TEST(GeneralConfig, SolarDayAndLocation) {
  const Config config = loadConfigFromString(SOLAR_DAY_AND_LOCATION_PROVIDED);

  const SolarDay expectedSolarDay = {
      .sunrise = convertTimeStringToTimeFromMidnightUnchecked("10:00"),
      .sunset = convertTimeStringToTimeFromMidnightUnchecked("20:00")};

  EXPECT_TRUE(config.solarDayProvider.getSolarDay() == expectedSolarDay);
}

TEST(GeneralConfig, SolarDayOnly) {
  const Config config = loadConfigFromString(SOLAR_DAY_ONLY);

  const SolarDay expectedSolarDay = {
      .sunrise = convertTimeStringToTimeFromMidnightUnchecked("10:00"),
      .sunset = convertTimeStringToTimeFromMidnightUnchecked("20:00")};

  EXPECT_TRUE(config.solarDayProvider.getSolarDay() == expectedSolarDay);
}

// If unable to create a valid solar day, use the default
TEST(GeneralConfig, SolarDayJustSunrise) {
  const Config config = loadConfigFromString(SOLAR_DAY_JUST_SUNRISE);

  EXPECT_TRUE(solarDayIsDefault(config.solarDayProvider.getSolarDay()));
}

// If unable to create a valid solar day, use the default
TEST(GeneralConfig, SolarDayJustSunset) {
  const Config config = loadConfigFromString(SOLAR_DAY_JUST_SUNSET);

  EXPECT_TRUE(solarDayIsDefault(config.solarDayProvider.getSolarDay()));
}

// If solar day informatino is incomplete, but has a location, should use the
// location to get the solar day
TEST(GeneralConfig, SolarDayIncompleteButHasLocation) {
  const Config config =
      loadConfigFromString(INCOMPLETE_SOLAR_DAY_AND_COMPLETE_LOCATION);

  const LocationInfo testLocation = {
      .latitudeAndLongitude = std::make_pair(TEST_LATITUDE, TEST_LONGITUDE),
      .useLatitudeAndLongitudeOverLocationSearch = true};

  EXPECT_TRUE(solarDayMatchesSolarDayForLocation(
      config.solarDayProvider.getSolarDay(), testLocation));
}
