#include "config.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

#include <yaml-cpp/yaml.h>

#include "yaml_helper.hpp"

namespace dynamic_paper {

static constexpr std::string METHOD_KEY = "method";
static constexpr std::string BACKGROUND_IMAGE_DIR_KEY = "image_dir";
static constexpr std::string HOOK_SCRIPT_KEY = "hook_script";

Config::Config(std::filesystem::path imageDir, BackgroundSetterMethod method,
               std::optional<std::filesystem::path> hookScript)
    : backgroundImageDir(imageDir), backgroundSetterMethod(method),
      hookScript(hookScript) {}

Config loadConfigFromFile(std::filesystem::path path) {
  if (!std::filesystem::exists(path)) {
    throw std::invalid_argument("No general config file exists at " +
                                path.string());
  }

  YAML::Node config = YAML::LoadFile(path.string());

  BackgroundSetterMethod method = parseOrUseDefault<BackgroundSetterMethod>(
      config, METHOD_KEY, ConfigDefaults::backgroundSetterMethod);

  std::filesystem::path backgroundImageDir =
      parseOrUseDefault<std::filesystem::path>(
          config, BACKGROUND_IMAGE_DIR_KEY, ConfigDefaults::backgroundImageDir);

  std::optional<std::filesystem::path> hookScript =
      parseOrUseDefault<std::optional<std::filesystem::path>>(
          config, HOOK_SCRIPT_KEY, std::nullopt);

  std::cout << (method == BackgroundSetterMethod::Script ? "script" : "wall")
            << std::endl;
  std::cout << backgroundImageDir << std::endl;
  std::cout << hookScript.has_value() << std::endl;
  std::cout << hookScript.value() << std::endl;
  // TODO broke

  return Config(path, BackgroundSetterMethod::Script, std::nullopt);
};

} // namespace dynamic_paper
