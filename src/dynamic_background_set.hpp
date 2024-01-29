#pragma once

#include <chrono>
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

struct TransitionInfo {
  std::chrono::seconds duration;
  unsigned int steps;

  explicit TransitionInfo(unsigned int duration, unsigned int steps)
      : duration(duration), steps(steps) {}
};

/** Type of `BackgroundSet` that shows different wallpapers at different times,
 * and changes over time*/
struct DynamicBackgroundData {
  std::filesystem::path dataDirectory;
  BackgroundSetMode mode;
  /** nullopt if does not transition. */
  std::optional<TransitionInfo> transition;

  BackgroundSetOrder order;
  std::vector<std::string> imageNames;
  /** each entry represents number seconds after 00:00 to do a transition */
  std::vector<time_t> times;

  DynamicBackgroundData(std::filesystem::path dataDirectory,
                        BackgroundSetMode mode,
                        std::optional<TransitionInfo> transition,
                        BackgroundSetOrder order,
                        std::vector<std::string> imageNames,
                        std::vector<time_t> times);

  /** Updates the background shown for the current time, and returns the amount
   * of seconds until the next event will be shown. */
  [[nodiscard]] std::chrono::seconds
  updateBackground(const Config &config) const;
};

} // namespace dynamic_paper
