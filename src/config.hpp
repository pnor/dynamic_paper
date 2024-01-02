#pragma once

#include <expected>
#include <filesystem>
#include <optional>
#include <type_traits>
#include <variant>

#include <yaml-cpp/yaml.h>

#include "logger.hpp"
#include "variant_visitor_templ.hpp"

// General Configuration

namespace dynamic_paper {

// ===== Sun Event Polling ===============

/** Type of ways to determine the position of the sun.
 *`Dummy` : For testing purposes
 *`Sunwait` : Use external program called sunwait
 */
enum class SunEventPollerMethod { Dummy, Sunwait };

// ===== Background Setter Method ===============

/** Base class that should be constructed */
struct BackgroundSetterMethodBase {};

/** Type of way to set the background of the desktop.
 * This uses a user defined script
 */
struct BackgroundSetterMethodScript : BackgroundSetterMethodBase {
  const std::filesystem::path script;
  explicit BackgroundSetterMethodScript(const std::filesystem::path script);
  bool operator==(const BackgroundSetterMethodScript &other) const {
    return script == other.script;
  };
};

/**
 * Type of way to set the background of the desktop
 * This uses an external program called "wallutils"
 */
struct BackgroundSetterMethodWallUtils : BackgroundSetterMethodBase {
  bool operator==(const BackgroundSetterMethodWallUtils &) const {
    return true;
  }
};

using BackgroundSetterMethod =
    std::variant<BackgroundSetterMethodScript, BackgroundSetterMethodWallUtils>;

/** Convert `method` to a representative string*/
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

/** Default values used if a config option is not specified in the user config
 */
struct ConfigDefaults {
  static constexpr std::string_view backgroundImageDir =
      "~/.local/share/dynamic_paper/images";
  static constexpr BackgroundSetterMethodWallUtils backgroundSetterMethod =
      BackgroundSetterMethodWallUtils();
  static constexpr SunEventPollerMethod sunEventPollerMethod =
      SunEventPollerMethod::Sunwait;
  static constexpr std::string_view imageCacheDirectory =
      "~/.cache/dynamic_paper";
  static constexpr LogLevel logLevel = LogLevel::INFO;

  ConfigDefaults() = delete;
  ConfigDefaults(ConfigDefaults &other) = delete;
  ConfigDefaults(ConfigDefaults &&other) = delete;
  ~ConfigDefaults() = delete;
};

/** Config options specified for user that control which images are used and how
 * they are shown*/
class Config {
public:
  std::filesystem::path backgroundImageDir;
  BackgroundSetterMethod backgroundSetterMethod;
  SunEventPollerMethod sunEventPollerMethod;
  std::optional<std::filesystem::path> hookScript;
  std::filesystem::path imageCacheDirectory;

  Config(std::filesystem::path imageDir, BackgroundSetterMethod setMethod,
         SunEventPollerMethod sunMethod,
         std::optional<std::filesystem::path> hookScript,
         std::filesystem::path imageCacheDirectory);
};

// ===== Loading config from files ====================

/** Loads the general config from a yaml file representing the config */
std::expected<Config, ConfigError> loadConfigFromYAML(const YAML::Node &config);

/** Laods the amount of logging from the general config file*/
LogLevel loadLoggingLevelFromYAML(const YAML::Node &config);

} // namespace dynamic_paper
