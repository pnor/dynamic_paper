#pragma once

#include <filesystem>
#include <optional>
#include <random>
#include <vector>

#include "background_set_enums.hpp"
#include "background_setter.hpp"

/** Static Background Sets show a wallpaper once and exit */

namespace dynamic_paper {

/** Type of `BackgroundSet` that shows a wallpaper once and exits */
struct StaticBackgroundData {
  std::filesystem::path dataDirectory;
  BackgroundSetMode mode;
  std::vector<std::string> imageNames;

  StaticBackgroundData(std::filesystem::path dataDirectory,
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
  const std::filesystem::path imagePath = dataDirectory / imageName;

  const tl::expected<void, BackgroundError> result =
      backgroundSetFunction(imagePath, mode, config.backgroundSetterMethod);

  if (!result.has_value()) {
    std::string modeString = backgroundSetModeString(this->mode);
    std::string methodString =
        backgroundSetterMethodString(config.backgroundSetterMethod);
    logWarning(
        "Encountered error in attempting to set the background for static "
        "background set. Image name was {} with mode {} using method {}",
        imageName, modeString, methodString);
  }

  if (config.hookScript.has_value()) {
    runHookCommand(config.hookScript.value(), imagePath);
  }
}

} // namespace dynamic_paper
