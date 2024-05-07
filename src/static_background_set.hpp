#pragma once

/** Static Background Sets show a wallpaper once and exit */

#include <filesystem>
#include <vector>

#include "background_set_enums.hpp"
#include "background_setter.hpp"
#include "config.hpp"
#include "hook_script_executor.hpp"

namespace dynamic_paper {

/** Type of `BackgroundSet` that shows a wallpaper once and exits */
struct StaticBackgroundData {
  std::filesystem::path imageDirectory;
  BackgroundSetMode mode;
  std::vector<std::string> imageNames;

  StaticBackgroundData(std::filesystem::path imageDirectory,
                       BackgroundSetMode mode,
                       std::vector<std::string> imageNames);

  /** Shows a background that is provided in this struct based on one of the
   * `imageNames`*/
  template <CanSetBackgroundTrait T>
  void show(const Config &config, T &&backgroundSetFunction) const;
};

// ===== Definition ===============

namespace detail {

/** Returns a random number in range [0..max)*/
size_t randomNumber(size_t max);

} // namespace detail

template <CanSetBackgroundTrait T>
void StaticBackgroundData::show(const Config &config,
                                T &&backgroundSetFunction) const {
  logTrace("Showing static background");
  logAssert(!imageNames.empty(),
            "Static background cannot show with no images");

  const std::string imageName =
      imageNames.at(detail::randomNumber(imageNames.size()));
  const std::filesystem::path imagePath = imageDirectory / imageName;

  // TODO forwward?
  backgroundSetFunction(imagePath, mode);

  if (config.hookScript.has_value()) {
    tl::expected<void, HookError> hookResult =
        runHookScript(config.hookScript.value(), imagePath);

    if (!hookResult.has_value()) {
      logError("Error relating to forking occured when running hook script");
    }
  }
}

} // namespace dynamic_paper
