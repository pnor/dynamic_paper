#include "config.hpp"

#include <fstream>
#include <iostream>

#include <yaml-cpp/yaml.h>

namespace dynamic_paper {

Config::Config(std::filesystem::path imageDir, BackgroundSetterMethod method,
               std::optional<std::filesystem::path> hookScript)
    : backgroundImageDir(imageDir), backgroundSetterMethod(method),
      hookScript(hookScript) {}

Config loadConfigFromFile(std::filesystem::path path) {

  YAML::Node config = YAML::LoadFile(path.string());
  std::cout << config["method"].as<std::string>() << std::endl;
  std::cout << config["image_dir"].as<std::string>() << std::endl;
  std::cout << config["hook_script"].as<std::string>() << std::endl;

  return Config(path, BackgroundSetterMethod::Script, std::nullopt);
}

} // namespace dynamic_paper
