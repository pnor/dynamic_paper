#pragma once

#include "background_setter.hpp"
#include "file_util.hpp"

/** Default values used throughout */

namespace dynamic_paper {
/** Default values used if a config option is not specified in the user config
 */
struct ConfigDefaults {
  static constexpr BackgroundSetterMethodWallUtils backgroundSetterMethod =
      BackgroundSetterMethodWallUtils();
  static constexpr SunEventPollerMethod sunEventPollerMethod =
      SunEventPollerMethod::Sunwait;
  static constexpr LogLevel logLevel = LogLevel::INFO;

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
  ConfigDefaults(ConfigDefaults &other) = delete;
  ConfigDefaults(ConfigDefaults &&other) = delete;
  ~ConfigDefaults() = delete;
};

constexpr std::string_view DEFAULT_CONFIG_FILE_NAME =
    "~/.config/dynamic_paper/config.yaml";

constexpr std::string_view DEFAULT_CONFIG_FILE_CONTENTS = R"""(method: wallutils
sun_poller: sunwait
)""";

} // namespace dynamic_paper
