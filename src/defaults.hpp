#pragma once

#include "background_setter.hpp"

/** Default values used throughout */

namespace dynamic_paper {
/** Default values used if a config option is not specified in the user config
 */
struct ConfigDefaults {
  static constexpr std::string_view backgroundSetConfigFile =
      "~/.local/share/dynamic_paper/background_sets.yaml";
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

constexpr std::string_view DEFAULT_CONFIG_FILE = R""""(
method: wallutils
sun_poller: sunwait
image_dir: ./an_image_dir
)"""";

} // namespace dynamic_paper
