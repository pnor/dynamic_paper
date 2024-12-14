#pragma once

/**
 * Parsing and usage of the general config
 */

#include <filesystem>
#include <optional>

#include <tl/expected.hpp>
#include <yaml-cpp/yaml.h>

#include "logger.hpp"
#include "solar_day_provider.hpp"

namespace dynamic_paper {

// ===== Config ===============

/** Config options specified for user that control which images are used and how
 * they are shown*/
class Config {
public:
  /** Location to where the file which Background Sets are loaded from*/
  std::filesystem::path backgroundSetConfigFile;
  /** Location of script that is called after a background is set*/
  std::optional<std::filesystem::path> hookScript;
  /** Location of the directory cached images created to transition between
   * backgrounds is kept*/
  std::filesystem::path imageCacheDirectory;

  /**
   * Used to get the solar day of the user
   */
  SolarDayProvider solarDayProvider;

  Config(std::filesystem::path backgroundSetConfigFile,
         std::optional<std::filesystem::path> hookScript,
         std::filesystem::path imageCacheDirectory,
         SolarDayProvider solarDayProvider);
};

// ===== Loading config from files ====================

/** Loads the general config from a yaml file representing the config */
Config loadConfigFromYAML(const YAML::Node &config, bool findLocationOverHttp);

/** Laods just the logging related information from the general config file*/
std::pair<LogLevel, std::filesystem::path>
loadLoggingInfoFromYAML(const YAML::Node &config);

} // namespace dynamic_paper
