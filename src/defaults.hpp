#pragma once

/** Default values used throughout */

#include "background_set_enums.hpp"
#include "file_util.hpp"
#include "time_util.hpp"

namespace dynamic_paper {

/**
 * Default values used if a config option is not specified in the user config
 */
struct ConfigDefaults {
  static constexpr LogLevel logLevel = LogLevel::INFO;
  static constexpr LocationInfo locationInfo = {
      .latitudeAndLongitude = std::make_pair(40.730610, -73.935242),
      .useLatitudeAndLongitudeOverLocationSearch = false};
  static constexpr SolarDay solarDay = {
      .sunrise = convertTimeStringToTimeFromMidnightUnchecked("09:00"),
      .sunset = convertTimeStringToTimeFromMidnightUnchecked("21:00")};

  static inline std::string logFileName() {
    return (getHomeDirectory() /
            ".local/share/dynamic_paper/dynamic_paper.log");
  }

  static inline std::string backgroundSetConfigFile() {
    return (getHomeDirectory() /
            std::filesystem::path(
                ".local/share/dynamic_paper/background_sets.yaml"))
        .string();
  }

  static inline std::string imageCacheDirectory() {
    return (getHomeDirectory() / std::filesystem::path(".cache/dynamic_paper"))
        .string();
  }

  ConfigDefaults() = delete;
  ConfigDefaults &operator=(const ConfigDefaults &) = delete;
  ConfigDefaults &operator=(ConfigDefaults &&) = delete;
  ConfigDefaults(const ConfigDefaults &other) = delete;
  ConfigDefaults(ConfigDefaults &&other) = delete;
  ~ConfigDefaults() = delete;
};

struct BackgroundSetDefaults {
  static constexpr BackgroundSetMode mode = BackgroundSetMode::Center;
  static constexpr BackgroundSetOrder order = BackgroundSetOrder::Linear;
  static constexpr unsigned int transitionSteps = 5;

  BackgroundSetDefaults() = delete;
  BackgroundSetDefaults &operator=(const BackgroundSetDefaults &) = delete;
  BackgroundSetDefaults &operator=(BackgroundSetDefaults &&) = delete;
  BackgroundSetDefaults(const BackgroundSetDefaults &other) = delete;
  BackgroundSetDefaults(BackgroundSetDefaults &&other) = delete;
  ~BackgroundSetDefaults() = delete;
};

constexpr std::string_view DEFAULT_CONFIG_FILE_NAME =
    "~/.config/dynamic_paper/config.yaml";

constexpr std::string_view DEFAULT_CONFIG_FILE_CONTENTS = R"""(method: wallutils
sun_poller: sunwait
)""";

} // namespace dynamic_paper
