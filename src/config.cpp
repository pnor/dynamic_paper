#include "config.hpp"

#include <utility>

#include <yaml-cpp/yaml.h>

#include "defaults.hpp"
#include "yaml_helper.hpp"

namespace dynamic_paper {

namespace {

using ExpectedMethod =
    tl::expected<BackgroundSetterMethod, BackgroundSetterMethodError>;

constexpr std::string_view METHOD_KEY = "method";
constexpr std::string_view SUN_POLL_METHOD_KEY = "sun_poller";
constexpr std::string_view BACKGROUND_SET_CONFIG_FILE = "background_config";
constexpr std::string_view METHOD_SCRIPT_KEY = "method_script";
constexpr std::string_view HOOK_SCRIPT_KEY = "hook_script";
constexpr std::string_view IMAGE_CACHE_DIR_KEY = "cache_dir";
constexpr std::string_view LOGGING_KEY = "logging_level";
constexpr std::string_view LOG_FILE_KEY = "log_file";
constexpr std::string_view LATITUDE_KEY = "latitude";
constexpr std::string_view LONGITUDE_KEY = "longitude";
constexpr std::string_view USE_CONFIG_FILE_LOCATION_KEY =
    "use_config_file_location";

ExpectedMethod handleMethodError(const ExpectedMethod &expectedMethod) {
  if (expectedMethod.has_value()) {
    return expectedMethod;
  }

  switch (expectedMethod.error()) {
  case BackgroundSetterMethodError::NoMethodInYAML: {
    logInfo("No method in general config so defaulting to Wallutils ");
    return BackgroundSetterMethodWallUtils();
  }
  case BackgroundSetterMethodError::InvalidMethod: {
    logInfo("Invalid method in general config so defaulting to Wallutils ");
    return BackgroundSetterMethodWallUtils();
  }
  case BackgroundSetterMethodError::NoScriptProvided: {
    return tl::unexpected(BackgroundSetterMethodError::NoScriptProvided);
  }
  }

  logAssert(false, "Reached impossible case when handling expected method "
                   "return type when parsing method from config!");
  return tl::unexpected(BackgroundSetterMethodError::InvalidMethod);
}

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

BackgroundSetterMethodScript::BackgroundSetterMethodScript(
    std::filesystem::path scriptPath)
    : script(std::move(scriptPath)) {}

Config::Config(std::filesystem::path backgroundSetConfigFile,
               BackgroundSetterMethod setMethod, SunEventPollerMethod sunMethod,
               std::optional<std::filesystem::path> hookScript,
               std::filesystem::path imageCacheDirectory,
               LocationInfo locationInfo)
    : backgroundSetConfigFile(std::move(backgroundSetConfigFile)),
      backgroundSetterMethod(std::move(setMethod)),
      sunEventPollerMethod(sunMethod), hookScript(std::move(hookScript)),
      imageCacheDirectory(std::move(imageCacheDirectory)),
      locationInfo(std::move(locationInfo)) {}

tl::expected<Config, ConfigError> loadConfigFromYAML(const YAML::Node &config) {

  const ExpectedMethod parsedExpectedMethod =
      parseBackgroundSetterMethod(config, METHOD_KEY, METHOD_SCRIPT_KEY);
  const ExpectedMethod expectedMethod = handleMethodError(parsedExpectedMethod);
  if (!expectedMethod.has_value()) {
    return tl::unexpected(ConfigError::MethodParsingError);
  }

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

  return Config(backgroundSetConfigFile, expectedMethod.value(), sunMethod,
                hookScript, imageCacheDir, locationInfo);
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
