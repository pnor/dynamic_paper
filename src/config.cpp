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

BackgroundSetterMethodScript::BackgroundSetterMethodScript(
    const std::filesystem::path scriptPath)
    : hookScript(scriptPath) {}

Config::Config(std::filesystem::path imageDir, BackgroundSetterMethod setMethod,
               SunEventPollerMethod sunMethod,
               std::optional<std::filesystem::path> hookScript)
    : backgroundImageDir(imageDir), backgroundSetterMethod(setMethod),
      sunEventPollerMethod(sunMethod), hookScript(hookScript) {}

std::expected<Config, ConfigError> loadConfigFromYAML(YAML::Node config) {
  // TODO hook script is NOT script...
  // make it so that
  // if script is set, default to
  // if mode is wallutil do that
  // if both script and wallutils, do default to ..?

  const std::expected<BackgroundSetterMethod, BackgroundSetterMethodError>
      expectedMethod =
          parseBackgroundSetterMethod(config, METHOD_KEY, METHOD_SCRIPT_KEY);

  if (!expectedMethod.has_value()) {
    return std::unexpected(ConfigError::MethodParsingError);
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

  return Config(backgroundImageDir, expectedMethod.value(), sunMethod,
                hookScript);
};

} // namespace dynamic_paper
