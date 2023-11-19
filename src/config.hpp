#pragma once

#include <expected>
#include <filesystem>
#include <optional>
#include <type_traits>
#include <variant>

#include <yaml-cpp/yaml.h>

#include "variant_visitor_templ.hpp"

// General Configuration

namespace dynamic_paper {

// ===== Sun Event Polling ===============
enum class SunEventPollerMethod { Dummy, Sunwait };

// ===== Background Setter Method ===============
struct BackgroundSetterMethodBase {};
struct BackgroundSetterMethodScript : BackgroundSetterMethodBase {
  const std::filesystem::path script;
  explicit BackgroundSetterMethodScript(const std::filesystem::path script);
  bool operator==(const BackgroundSetterMethodScript &other) const {
    return script == other.script;
  };
};
struct BackgroundSetterMethodWallUtils : BackgroundSetterMethodBase {
  bool operator==(const BackgroundSetterMethodWallUtils &) const {
    return true;
  }
};
using BackgroundSetterMethod =
    std::variant<BackgroundSetterMethodScript, BackgroundSetterMethodWallUtils>;

constexpr std::string
backgroundSetterMethodString(const BackgroundSetterMethod &method) {
  std::string returnString;

  std::visit(overloaded{[&returnString](const BackgroundSetterMethodScript &) {
                          returnString = "script";
                        },
                        [&returnString](const BackgroundSetterMethodWallUtils) {
                          returnString = "wallutils";
                        }},
             method);

  return returnString;
}

// ===== Config ===============

enum class ConfigError { MethodParsingError };

struct ConfigDefaults {
  static constexpr std::string_view backgroundImageDir =
      "~/.local/share/dynamic_paper/images";
  static constexpr BackgroundSetterMethodWallUtils backgroundSetterMethod =
      BackgroundSetterMethodWallUtils();
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

  Config(std::filesystem::path imageDir, BackgroundSetterMethod setMethod,
         SunEventPollerMethod sunMethod,
         std::optional<std::filesystem::path> hookScript);
};

std::expected<Config, ConfigError> loadConfigFromYAML(YAML::Node config);

} // namespace dynamic_paper
