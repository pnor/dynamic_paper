#include "config.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

#include <yaml-cpp/yaml.h>

#include "yaml_helper.hpp"

namespace dynamic_paper {

static constexpr std::string METHOD_KEY = "method";
static constexpr std::string SUN_POLL_METHOD_KEY = "sun_poller";
static constexpr std::string BACKGROUND_IMAGE_DIR_KEY = "image_dir";
static constexpr std::string METHOD_SCRIPT_KEY = "method_script";
static constexpr std::string HOOK_SCRIPT_KEY = "hook_script";
static constexpr std::string IMAGE_CACHE_DIR_KEY = "cache_dir";

BackgroundSetterMethodScript::BackgroundSetterMethodScript(
    const std::filesystem::path scriptPath)
    : script(scriptPath) {}

Config::Config(std::filesystem::path imageDir, BackgroundSetterMethod setMethod,
               SunEventPollerMethod sunMethod,
               std::optional<std::filesystem::path> hookScript,
               std::filesystem::path imageCacheDirectory)
    : backgroundImageDir(imageDir), backgroundSetterMethod(setMethod),
      sunEventPollerMethod(sunMethod), hookScript(hookScript),
      imageCacheDirectory(imageCacheDirectory) {}

std::expected<Config, ConfigError> loadConfigFromYAML(YAML::Node config) {
  std::expected<BackgroundSetterMethod, BackgroundSetterMethodError>
      expectedMethod =
          parseBackgroundSetterMethod(config, METHOD_KEY, METHOD_SCRIPT_KEY);

  if (!expectedMethod.has_value()) {
    switch (expectedMethod.error()) {
    case BackgroundSetterMethodError::NoMethodInYAML: {
      logInfo("No method in general config so defaulting to Wallutils ");
      expectedMethod = BackgroundSetterMethodWallUtils();
      break;
    }
    default: {
      return std::unexpected(ConfigError::MethodParsingError);
    }
    }
  }

  SunEventPollerMethod sunMethod =
      generalConfigParseOrUseDefault<SunEventPollerMethod>(
          config, SUN_POLL_METHOD_KEY, ConfigDefaults::sunEventPollerMethod);

  std::filesystem::path backgroundImageDir =
      generalConfigParseOrUseDefault<std::filesystem::path>(
          config, BACKGROUND_IMAGE_DIR_KEY, ConfigDefaults::backgroundImageDir);

  std::optional<std::filesystem::path> hookScript =
      generalConfigParseOrUseDefault<std::optional<std::filesystem::path>>(
          config, HOOK_SCRIPT_KEY, std::nullopt);

  std::filesystem::path imageCacheDir =
      generalConfigParseOrUseDefault<std::filesystem::path>(
          config, IMAGE_CACHE_DIR_KEY, ConfigDefaults::imageCacheDirectory);
  // TODO add test case for this

  return Config(backgroundImageDir, expectedMethod.value(), sunMethod,
                hookScript, imageCacheDir);
};

} // namespace dynamic_paper
