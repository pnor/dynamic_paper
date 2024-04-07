#pragma once

/**
 * Parsing and usage of the general config
 */

#include <filesystem>
#include <optional>
#include <variant>

#include <tl/expected.hpp>
#include <yaml-cpp/yaml.h>

#include "logger.hpp"
#include "variant_visitor_templ.hpp"

namespace dynamic_paper {

// ===== Sun Event Polling ===============

/**
 * TODO rework since dummy needs to be like manually setting; remove and put
 *into config? also reomve 'sunwait'
 *
 *Type of ways to determine the position of the sun. `Dummy` : For
 *testing purposes `Sunwait` : Use external program called sunwait
 */
enum class SunEventPollerMethod { Dummy, Sunwait };

// ===== Location Info ===============

/**
 * Describes how to infer the user's position
 *
 * If `useLocationInfoOverSearch` is true, will not try and search for the
 * user's position and use `latitudeAndLongitude` as the position.
 */
struct LocationInfo {
  std::pair<double, double> latitudeAndLongitude;
  /**
   * If true, will *not* search for the
   * user's position and instead use `latitudeAndLongitude` as the position.
   */
  bool useLocationInfoOverSearch;
};

// ===== Config ===============

enum class ConfigError { MethodParsingError };

/** Config options specified for user that control which images are used and how
 * they are shown*/
class Config {
public:
  /** Location to where the file which Background Sets are loaded from*/
  std::filesystem::path backgroundSetConfigFile;
  /** Method used to determine the location of the sun in the sky */
  SunEventPollerMethod sunEventPollerMethod;
  /** Location of script that is called after a background is set*/
  std::optional<std::filesystem::path> hookScript;
  /** Location of the directory cached images created to transition between
   * backgrounds is kept*/
  std::filesystem::path imageCacheDirectory;
  /**
   * Location to use when determining time of sunrise and sunset
   */
  LocationInfo locationInfo;

  Config(std::filesystem::path backgroundSetConfigFile,
         SunEventPollerMethod sunMethod,
         std::optional<std::filesystem::path> hookScript,
         std::filesystem::path imageCacheDirectory, LocationInfo locationInfo);
};

// ===== Loading config from files ====================

/** Loads the general config from a yaml file representing the config */
tl::expected<Config, ConfigError> loadConfigFromYAML(const YAML::Node &config);

/** Laods just the logging related information from the general config file*/
std::pair<LogLevel, std::filesystem::path>
loadLoggingInfoFromYAML(const YAML::Node &config);

} // namespace dynamic_paper
