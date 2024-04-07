#include "config.hpp"

#include <utility>

#include <yaml-cpp/yaml.h>

#include "defaults.hpp"
#include "yaml_helper.hpp"

namespace dynamic_paper {

namespace {

constexpr std::string_view SUN_POLL_METHOD_KEY = "sun_poller"; // TODO remove
constexpr std::string_view BACKGROUND_SET_CONFIG_FILE = "background_config";
constexpr std::string_view METHOD_SCRIPT_KEY = "method_script"; // TODO remove
constexpr std::string_view HOOK_SCRIPT_KEY = "hook_script";
constexpr std::string_view IMAGE_CACHE_DIR_KEY = "cache_dir";
constexpr std::string_view LOGGING_KEY = "logging_level";
constexpr std::string_view LOG_FILE_KEY = "log_file";
constexpr std::string_view LATITUDE_KEY = "latitude";
constexpr std::string_view LONGITUDE_KEY = "longitude";
constexpr std::string_view USE_CONFIG_FILE_LOCATION_KEY =
    "use_config_file_location";

LocationInfo createLocationInfoFromParsedFields(
    const std::optional<double> optLatitude,
    const std::optional<double> optLongitude,
    const std::optional<bool> optUseLocationInfoOverSearch) {
  if (!(optLatitude.has_value() && optLongitude.has_value())) {
    return ConfigDefaults::locationInfo;
  }

  return {.latitudeAndLongitude =
              std::make_pair(optLatitude.value(), optLongitude.value()),
          .useLocationInfoOverSearch =
              optUseLocationInfoOverSearch.value_or(false)};
}

} // namespace

// ===== Header ===============

Config::Config(std::filesystem::path backgroundSetConfigFile,
               SunEventPollerMethod sunMethod,
               std::optional<std::filesystem::path> hookScript,
               std::filesystem::path imageCacheDirectory,
               LocationInfo locationInfo)
    : backgroundSetConfigFile(std::move(backgroundSetConfigFile)),
      sunEventPollerMethod(sunMethod), hookScript(std::move(hookScript)),
      imageCacheDirectory(std::move(imageCacheDirectory)),
      locationInfo(std::move(locationInfo)) {}

tl::expected<Config, ConfigError> loadConfigFromYAML(const YAML::Node &config) {
  const auto sunMethod = generalConfigParseOrUseDefault<SunEventPollerMethod>(
      config, SUN_POLL_METHOD_KEY, ConfigDefaults::sunEventPollerMethod);

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
  LocationInfo locationInfo = createLocationInfoFromParsedFields(
      optLatitude, optLongitude, optUseLocationInfoOverSearch);

  return Config(backgroundSetConfigFile, sunMethod, hookScript, imageCacheDir,
                locationInfo);
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
