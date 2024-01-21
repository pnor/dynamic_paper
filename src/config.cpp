#include "config.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

#include <yaml-cpp/yaml.h>

#include "defaults.hpp"
#include "globals.hpp"
#include "yaml_helper.hpp"

namespace dynamic_paper {

static constexpr std::string_view METHOD_KEY = "method";
static constexpr std::string_view SUN_POLL_METHOD_KEY = "sun_poller";
static constexpr std::string_view BACKGROUND_SET_CONFIG_FILE =
    "background_config";
static constexpr std::string_view METHOD_SCRIPT_KEY = "method_script";
static constexpr std::string_view HOOK_SCRIPT_KEY = "hook_script";
static constexpr std::string_view IMAGE_CACHE_DIR_KEY = "cache_dir";
static constexpr std::string_view LOGGING_KEY = "logging_level";

using ExpectedMethod =
    tl::expected<BackgroundSetterMethod, BackgroundSetterMethodError>;

static ExpectedMethod handleMethodError(const ExpectedMethod &expectedMethod) {
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

// ===== Header ===============

BackgroundSetterMethodScript::BackgroundSetterMethodScript(
    const std::filesystem::path scriptPath)
    : script(scriptPath) {}

Config::Config(std::filesystem::path backgroundSetConfigFile,
               BackgroundSetterMethod setMethod, SunEventPollerMethod sunMethod,
               std::optional<std::filesystem::path> hookScript,
               std::filesystem::path imageCacheDirectory)
    : backgroundSetConfigFile(backgroundSetConfigFile),
      backgroundSetterMethod(setMethod), sunEventPollerMethod(sunMethod),
      hookScript(hookScript), imageCacheDirectory(imageCacheDirectory) {}

tl::expected<Config, ConfigError> loadConfigFromYAML(const YAML::Node &config) {

  const ExpectedMethod parsedExpectedMethod =
      parseBackgroundSetterMethod(config, METHOD_KEY, METHOD_SCRIPT_KEY);
  const ExpectedMethod expectedMethod = handleMethodError(parsedExpectedMethod);
  if (!expectedMethod.has_value()) {
    return tl::unexpected(ConfigError::MethodParsingError);
  }

  SunEventPollerMethod sunMethod =
      generalConfigParseOrUseDefault<SunEventPollerMethod>(
          config, SUN_POLL_METHOD_KEY, ConfigDefaults::sunEventPollerMethod);

  std::filesystem::path backgroundSetConfigFile =
      generalConfigParseOrUseDefault<std::filesystem::path>(
          config, BACKGROUND_SET_CONFIG_FILE,
          ConfigDefaults::backgroundSetConfigFile);

  std::optional<std::filesystem::path> hookScript =
      generalConfigParseOrUseDefault<std::optional<std::filesystem::path>>(
          config, HOOK_SCRIPT_KEY, std::nullopt);

  std::filesystem::path imageCacheDir =
      generalConfigParseOrUseDefault<std::filesystem::path>(
          config, IMAGE_CACHE_DIR_KEY, ConfigDefaults::imageCacheDirectory);

  return Config(backgroundSetConfigFile, expectedMethod.value(), sunMethod,
                hookScript, imageCacheDir);
};

LogLevel loadLoggingLevelFromYAML(const YAML::Node &config) {
  return generalConfigParseOrUseDefault<LogLevel>(config, LOGGING_KEY,
                                                  ConfigDefaults::logLevel);
}

} // namespace dynamic_paper
