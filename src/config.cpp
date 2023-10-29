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
static constexpr std::string HOOK_SCRIPT_KEY = "hook_script";

Config::Config(std::filesystem::path imageDir, BackgroundSetterMethod method,
               SunEventPollerMethod sunMethod,
               std::optional<std::filesystem::path> hookScript)
    : backgroundImageDir(imageDir), backgroundSetterMethod(method),
      sunEventPollerMethod(sunMethod), hookScript(hookScript) {}

Config loadConfigFromYAML(YAML::Node config) {
  BackgroundSetterMethod method =
      generalConfigParseOrUseDefault<BackgroundSetterMethod>(
          config, METHOD_KEY, ConfigDefaults::backgroundSetterMethod);

  SunEventPollerMethod sunMethod =
      generalConfigParseOrUseDefault<SunEventPollerMethod>(
          config, SUN_POLL_METHOD_KEY, ConfigDefaults::sunEventPollerMethod);

  std::filesystem::path backgroundImageDir =
      generalConfigParseOrUseDefault<std::filesystem::path>(
          config, BACKGROUND_IMAGE_DIR_KEY, ConfigDefaults::backgroundImageDir);

  std::optional<std::filesystem::path> hookScript =
      generalConfigParseOrUseDefault<std::optional<std::filesystem::path>>(
          config, HOOK_SCRIPT_KEY, std::nullopt);

  return Config(backgroundImageDir, method, sunMethod, hookScript);
};

} // namespace dynamic_paper
