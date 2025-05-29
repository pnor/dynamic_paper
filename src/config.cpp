#include "config.hpp"

#include <filesystem>
#include <optional>
#include <string_view>
#include <utility>

#include <yaml-cpp/node/node.h>

#include "constants.hpp"
#include "defaults.hpp"
#include "file_util.hpp"
#include "location_info.hpp"
#include "logger.hpp"
#include "solar_day.hpp"
#include "solar_day_provider.hpp"
#include "time_from_midnight.hpp"
#include "yaml_helper.hpp"

namespace dynamic_paper {

namespace {

LocationInfo createLocationInfoFromParsedFields(
    const std::optional<double> optLatitude, const std::optional<double> optLongitude,
    const std::optional<bool> optUseLatitudeAndLongitudeOverLocationSearch) {
  if (!(optLatitude.has_value() && optLongitude.has_value())) {
    return ConfigDefaults::locationInfo;
  }

  return {.latitudeAndLongitude =
              std::pair<double, double>(optLatitude.value(), optLongitude.value()),
          .useLatitudeAndLongitudeOverLocationSearch =
              optUseLatitudeAndLongitudeOverLocationSearch.value_or(false)};
}

SolarDayProvider createSolarDayProviderFromParsedFields(
    const std::optional<double> optLatitude, const std::optional<double> optLongitude,
    const std::optional<bool> optUseLatitudeAndLongitudeOverLocationSearch,
    const std::optional<TimeFromMidnight> optSunriseTime,
    const std::optional<TimeFromMidnight> optSunsetTime) {
  const bool canCreateLocationInfo = optLatitude.has_value() && optLongitude.has_value();
  const bool canCreateSolarDay = optSunriseTime.has_value() && optSunsetTime.has_value();

  // enough info for both
  if (canCreateLocationInfo && canCreateSolarDay) {
    logDebug("Determining solar day using values provided in config: "
             "sunrise={} sunset={}",
             optSunriseTime.value(), optSunsetTime.value());
    return {SolarDay{.sunrise = optSunriseTime.value(), .sunset = optSunsetTime.value()}};
  }

  // not enough info for either
  if (!canCreateLocationInfo && !canCreateSolarDay) {
    logDebug("Determining solar day using default value");
    return {ConfigDefaults::solarDay};
  }

  // only info for location
  if (canCreateLocationInfo) {
    logInfo("Determining solar day using latitude and longitude: latitude={} "
            "longitude={}",
            optLatitude.value(), optLongitude.value());
    return {createLocationInfoFromParsedFields(optLatitude, optLongitude,
                                               optUseLatitudeAndLongitudeOverLocationSearch)};
  }

  // only info for solar day
  logDebug("Determining solar day using values provided in config: "
           "sunrise={} sunset={}",
           optSunriseTime.value(), optSunsetTime.value());
  return {SolarDay{.sunrise = optSunriseTime.value(), .sunset = optSunsetTime.value()}};
}

} // namespace

// ===== Header ===============

Config::Config(std::filesystem::path backgroundSetConfigFile,
               std::optional<std::filesystem::path> hookScript,
               std::filesystem::path imageCacheDirectory, BackgroundSetMethod method,
               SolarDayProvider solarDayProvider)
    : backgroundSetConfigFile(std::move(backgroundSetConfigFile)),
      hookScript(std::move(hookScript)), imageCacheDirectory(std::move(imageCacheDirectory)),
      method(method), solarDayProvider(std::move(solarDayProvider)) {}

Config loadConfigFromYAML(const YAML::Node &config, const bool findLocationOverHttp) {
  auto backgroundSetConfigFile = generalConfigParseOrUseDefault<std::filesystem::path>(
      config, BACKGROUND_SET_CONFIG_FILE, ConfigDefaults::backgroundSetConfigFile());
  backgroundSetConfigFile = expandPath(backgroundSetConfigFile);

  auto hookScript = generalConfigParseOrUseDefault<std::optional<std::filesystem::path>>(
      config, HOOK_SCRIPT_KEY, std::nullopt);
  if (hookScript.has_value()) {
    hookScript = expandPath(hookScript.value());
  }

  auto imageCacheDir = generalConfigParseOrUseDefault<std::filesystem::path>(
      config, IMAGE_CACHE_DIR_KEY, ConfigDefaults::imageCacheDirectory());
  imageCacheDir = expandPath(imageCacheDir);

  const auto optLatitude =
      generalConfigParseOrUseDefault<std::optional<double>>(config, LATITUDE_KEY, std::nullopt);
  const auto optLongitude =
      generalConfigParseOrUseDefault<std::optional<double>>(config, LONGITUDE_KEY, std::nullopt);
  const auto optUseLocationInfoOverSearch = generalConfigParseOrUseDefault<std::optional<bool>>(
      config, USE_CONFIG_FILE_LOCATION_KEY, std::nullopt);
  const auto optSunriseTime = generalConfigParseOrUseDefault<std::optional<TimeFromMidnight>>(
      config, SUNRISE_TIME_KEY, std::nullopt);
  const auto optSunsetTime = generalConfigParseOrUseDefault<std::optional<TimeFromMidnight>>(
      config, SUNSET_TIME_KEY, std::nullopt);

  const auto method = generalConfigParseOrUseDefault<BackgroundSetMethod>(config, METHOD_KEY,
                                                                          ConfigDefaults::method);

  const SolarDayProvider solarDayProvider = createSolarDayProviderFromParsedFields(
      optLatitude, optLongitude, findLocationOverHttp ? optUseLocationInfoOverSearch : true,
      optSunriseTime, optSunsetTime);

  return {backgroundSetConfigFile, hookScript, imageCacheDir, method, solarDayProvider};
};

std::pair<LogLevel, std::filesystem::path> loadLoggingInfoFromYAML(const YAML::Node &config) {
  const auto level =
      generalConfigParseOrUseDefault<LogLevel>(config, LOGGING_KEY, ConfigDefaults::logLevel);

  const auto fileName = expandPath(generalConfigParseOrUseDefault<std::filesystem::path>(
      config, LOG_FILE_KEY, std::filesystem::path(ConfigDefaults::logFileName())));

  return std::make_pair(level, fileName);
}

} // namespace dynamic_paper
