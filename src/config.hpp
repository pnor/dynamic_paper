#pragma once

#include <filesystem>
#include <optional>

#include <yaml-cpp/yaml.h>

// General Configuration

namespace dynamic_paper {

enum class BackgroundSetterMethod { Script, WallUtils };
enum class SunEventPollerMethod { Sunwait };

struct ConfigDefaults {
  static constexpr std::string_view backgroundImageDir =
      "~/.local/share/dynamic_paper/images";
  static constexpr BackgroundSetterMethod backgroundSetterMethod =
      BackgroundSetterMethod::WallUtils;
  static constexpr SunEventPollerMethod sunEventPollerMethod =
      SunEventPollerMethod::Sunwait;

  ConfigDefaults() = delete;
  ConfigDefaults(ConfigDefaults &other) = delete;
  ConfigDefaults(ConfigDefaults &&other) = delete;
  ~ConfigDefaults() = delete;
};

class Config {
public:
  std::filesystem::path backgroundImageDir;
  BackgroundSetterMethod backgroundSetterMethod;
  SunEventPollerMethod sunEventPollerMethod;
  std::optional<std::filesystem::path> hookScript;

  Config(std::filesystem::path imageDir, BackgroundSetterMethod method,
         SunEventPollerMethod sunMethod,
         std::optional<std::filesystem::path> hookScript);
};

Config loadConfigFromYAML(YAML::Node config);

} // namespace dynamic_paper
