#pragma once

#include <filesystem>
#include <optional>
#include <type_traits>
#include <variant>

#include <tl/expected.hpp>
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

/** Config options specified for user that control which images are used and how
 * they are shown*/
class Config {
public:
  /** Location to where the file which Background Sets are loaded from*/
  std::filesystem::path backgroundSetConfigFile;
  /** Method used to set backgrounds as the wallpaper*/
  BackgroundSetterMethod backgroundSetterMethod;
  /** Method used to determine the location of the sun in the sky */
  SunEventPollerMethod sunEventPollerMethod;
  /** Location of script that is called after a background is set*/
  std::optional<std::filesystem::path> hookScript;
  /** Location of the directory cached images created to transition between
   * backgrounds is kept*/
  std::filesystem::path imageCacheDirectory;

  Config(std::filesystem::path backgroundSetConfigFile,
         BackgroundSetterMethod setMethod, SunEventPollerMethod sunMethod,
         std::optional<std::filesystem::path> hookScript,
         std::filesystem::path imageCacheDirectory);
};

// ===== Loading config from files ====================

/** Loads the general config from a yaml file representing the config */
tl::expected<Config, ConfigError> loadConfigFromYAML(const YAML::Node &config);

/** Laods the amount of logging from the general config file*/
LogLevel loadLoggingLevelFromYAML(const YAML::Node &config);

} // namespace dynamic_paper
