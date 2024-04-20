#include "config.hpp"

#include <utility>

#include <yaml-cpp/yaml.h>

#include "defaults.hpp"
#include "yaml_helper.hpp"

namespace dynamic_paper {

namespace {

constexpr std::string_view BACKGROUND_SET_CONFIG_FILE = "background_config";
constexpr std::string_view HOOK_SCRIPT_KEY = "hook_script";
constexpr std::string_view IMAGE_CACHE_DIR_KEY = "cache_dir";
constexpr std::string_view LOGGING_KEY = "logging_level";
constexpr std::string_view LOG_FILE_KEY = "log_file";
constexpr std::string_view LATITUDE_KEY = "latitude";
constexpr std::string_view LONGITUDE_KEY = "longitude";
constexpr std::string_view SUNRISE_TIME_KEY = "sunrise";
constexpr std::string_view SUNSET_TIME_KEY = "sunset";
constexpr std::string_view USE_CONFIG_FILE_LOCATION_KEY =
    "use_config_file_location";

LocationInfo createLocationInfoFromParsedFields(
    const std::optional<double> optLatitude,
    const std::optional<double> optLongitude,
    const std::optional<bool> optUseLatitudeAndLongitudeOverLocationSearch) {
  if (!(optLatitude.has_value() && optLongitude.has_value())) {
    return ConfigDefaults::locationInfo;
  }

  return {.latitudeAndLongitude =
              std::make_pair(optLatitude.value(), optLongitude.value()),
          .useLatitudeAndLongitudeOverLocationSearch =
              optUseLatitudeAndLongitudeOverLocationSearch.value_or(false)};
}

SolarDayProvider createSolarDayProviderFromParsedFields(
    const std::optional<double> optLatitude,
    const std::optional<double> optLongitude,
    const std::optional<bool> optUseLatitudeAndLongitudeOverLocationSearch,
    const std::optional<TimeFromMidnight> optSunriseTime,
    const std::optional<TimeFromMidnight> optSunsetTime) {
  const bool canCreateLocationInfo =
      optLatitude.has_value() && optLongitude.has_value();
  const bool canCreateSolarDay =
      optSunriseTime.has_value() && optSunsetTime.has_value();

  // enough info for both
  if (canCreateLocationInfo && canCreateSolarDay) {
    return {SolarDay{.sunrise = optSunriseTime.value(),
                     .sunset = optSunsetTime.value()}};
  }

  // not enough info for either
  if (!canCreateLocationInfo && !canCreateSolarDay) {
    return {ConfigDefaults::solarDay};
  }

  // only info for location
  if (canCreateLocationInfo) {
    return {createLocationInfoFromParsedFields(
        optLatitude, optLongitude,
        optUseLatitudeAndLongitudeOverLocationSearch)};
  }

  // only info for solar day
  return {SolarDay{.sunrise = optSunriseTime.value(),
                   .sunset = optSunsetTime.value()}};
}

} // namespace

// ===== Header ===============

Config::Config(std::filesystem::path backgroundSetConfigFile,
               std::optional<std::filesystem::path> hookScript,
               std::filesystem::path imageCacheDirectory,
               SolarDayProvider solarDayProvider)
    : backgroundSetConfigFile(std::move(backgroundSetConfigFile)),
      hookScript(std::move(hookScript)),
      imageCacheDirectory(std::move(imageCacheDirectory)),
      solarDayProvider(std::move(solarDayProvider)) {}

tl::expected<Config, ConfigError> loadConfigFromYAML(const YAML::Node &config) {
  auto backgroundSetConfigFile =
      generalConfigParseOrUseDefault<std::filesystem::path>(
          config, BACKGROUND_SET_CONFIG_FILE,
          ConfigDefaults::backgroundSetConfigFile());
  backgroundSetConfigFile = expandPath(backgroundSetConfigFile);

  auto hookScript =
      generalConfigParseOrUseDefault<std::optional<std::filesystem::path>>(
          config, HOOK_SCRIPT_KEY, std::nullopt);
  if (hookScript.has_value()) {
    hookScript = expandPath(hookScript.value());
  }

  auto imageCacheDir = generalConfigParseOrUseDefault<std::filesystem::path>(
      config, IMAGE_CACHE_DIR_KEY, ConfigDefaults::imageCacheDirectory());
  imageCacheDir = expandPath(imageCacheDir);

  const auto optLatitude =
      generalConfigParseOrUseDefault<std::optional<double>>(
          config, LATITUDE_KEY, std::nullopt);
  const auto optLongitude =
      generalConfigParseOrUseDefault<std::optional<double>>(
          config, LONGITUDE_KEY, std::nullopt);
  const auto optUseLocationInfoOverSearch =
      generalConfigParseOrUseDefault<std::optional<bool>>(
          config, USE_CONFIG_FILE_LOCATION_KEY, std::nullopt);
  const auto optSunriseTime =
      generalConfigParseOrUseDefault<std::optional<TimeFromMidnight>>(
          config, SUNRISE_TIME_KEY, std::nullopt);
  const auto optSunsetTime =
      generalConfigParseOrUseDefault<std::optional<TimeFromMidnight>>(
          config, SUNSET_TIME_KEY, std::nullopt);

  SolarDayProvider solarDayProvider = createSolarDayProviderFromParsedFields(
      optLatitude, optLongitude, optUseLocationInfoOverSearch, optSunriseTime,
      optSunsetTime);

  return Config(backgroundSetConfigFile, hookScript, imageCacheDir,
                solarDayProvider);
};

std::pair<LogLevel, std::filesystem::path>
loadLoggingInfoFromYAML(const YAML::Node &config) {
  const auto level = generalConfigParseOrUseDefault<LogLevel>(
      config, LOGGING_KEY, ConfigDefaults::logLevel);

  const auto fileName =
      expandPath(generalConfigParseOrUseDefault<std::filesystem::path>(
          config, LOG_FILE_KEY,
          std::filesystem::path(ConfigDefaults::logFileName())));

  return std::make_pair(level, fileName);
}

} // namespace dynamic_paper
