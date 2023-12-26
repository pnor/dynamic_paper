#pragma once

#include <filesystem>
#include <optional>
#include <vector>

#include "background_set_enums.hpp"
#include "background_setter.hpp"
#include "config.hpp"

/** Dynamic Background sets change wallpaper depending on the time of day. They
 * sleep until the next time to show a wallpaper is hit
 */

namespace dynamic_paper {

class DynamicBackgroundData {
public:
  std::filesystem::path dataDirectory;
  BackgroundSetMode mode;
  /** nullopt if does not transition. # seconds otherwise. */
  std::optional<unsigned int> transitionDuration;
  BackgroundSetOrder order;
  std::vector<std::string> imageNames;
  /** each entry represents number seconds after 00:00 to do a transition */
  std::vector<time_t> times;

  DynamicBackgroundData(std::filesystem::path dataDirectory,
                        BackgroundSetMode mode,
                        std::optional<unsigned int> transitionDuration,
                        BackgroundSetOrder order,
                        std::vector<std::string> imageNames,
                        std::vector<time_t> times);

  void show(const Config &config) const;
};

} // namespace dynamic_paper
