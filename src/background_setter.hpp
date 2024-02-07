#pragma once

#include <filesystem>
#include <string>

#include <tl/expected.hpp>

#include "background_set_enums.hpp"
#include "config.hpp"
#include "file_util.hpp"
#include "image_compositor.hpp"

/** Handles the logic of how the bckground is actually changed */

namespace dynamic_paper {

/** Errors that can occur when setting the background */
enum class BackgroundError {
  CommandError,
  CompositeImageError,
  SetBackgroundError,
  NoCacheDir,
};
/** Errors that occur when running a hook command */
enum class HookCommandError { CommandError };

/**
 * Trait for a type that contains a static function that can change the
 * background. Use as an abstraction to allow testing without changing global
 * state
 */
template <typename T>
concept CanSetBackgroundTrait =
    requires(const std::filesystem::path &imagePath, BackgroundSetMode mode,
             const BackgroundSetterMethod &method) {
      {
        T::setBackgroundToImage(imagePath, mode, method)
      } -> std::convertible_to<tl::expected<void, BackgroundError>>;
    };

/** Contains Functions that change the background of the system using
 * commands/function calls */
class BackgroundSetterTrait {
public:
  /**
   * Using a program specified by `method`, sets the background to the image in
   * `imagePath` using a display mode of `mode`
   */
  static tl::expected<void, BackgroundError>
  setBackgroundToImage(const std::filesystem::path &imagePath,
                       BackgroundSetMode mode,
                       const BackgroundSetterMethod &method);
};

/**
 * Using a program specified by `method`, changes the background from
 * `commonImageDirectory / beforeImageName` to `commonImageDirectory /
 * afterImageName`. This effect occurs for `duration` seconds and happens in
 * `numSteps` steps. Sets the background using a program specified by `method`,
 * with a display mode of `mode`. Uses `cacheDirectory` to store and get
 * composited images.
 *
 * Uses the external program imagemagick to create interpolated images.
 */
template <typename T>
  requires CanSetBackgroundTrait<T>
tl::expected<void, BackgroundError> lerpBackgroundBetweenImages(
    const std::filesystem::path &commonImageDirectory,
    const std::string &beforeImageName, const std::string &afterImageName,
    const std::filesystem::path &cacheDirectory, std::chrono::seconds duration,
    const unsigned int numSteps, BackgroundSetMode mode,
    const BackgroundSetterMethod &method);

/**
 * Runs the script at `hookScriptPath` on the image at `imagePath`.
 *
 * Used to run user defined hooks after the background changes.
 */
tl::expected<void, HookCommandError>
runHookCommand(const std::filesystem::path &hookScriptPath,
               const std::filesystem::path &imagePath);

// ===== Definition ==========

template <typename T>
  requires CanSetBackgroundTrait<T>
tl::expected<void, BackgroundError> lerpBackgroundBetweenImages(
    const std::filesystem::path &commonImageDirectory,
    const std::string &beforeImageName, const std::string &afterImageName,
    const std::filesystem::path &cacheDirectory,
    const std::chrono::seconds duration, const unsigned int numSteps,
    const BackgroundSetMode mode, const BackgroundSetterMethod &method) {

  const bool dirCreationResult = createDirectoryIfDoesntExist(cacheDirectory);
  if (!dirCreationResult) {
    return tl::make_unexpected(BackgroundError::NoCacheDir);
  }

  for (unsigned int i = 0; i < numSteps; i++) {
    const float percentageFloat =
        static_cast<float>(i) / static_cast<float>(numSteps);
    const unsigned int percentage = std::clamp(
        static_cast<unsigned int>(percentageFloat * 100.0), 0U, 100U);

    const tl::expected<std::filesystem::path, CompositeImageError>
        expectedCompositedImage =
            getCompositedImage(commonImageDirectory, beforeImageName,
                               afterImageName, cacheDirectory, percentage);

    if (!expectedCompositedImage.has_value()) {
      return tl::unexpected(BackgroundError::CompositeImageError);
    }

    const tl::expected<void, BackgroundError> backgroundResult =
        T::setBackgroundToImage(expectedCompositedImage.value(), mode, method);

    if (!backgroundResult.has_value()) {
      return tl::unexpected(BackgroundError::SetBackgroundError);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(duration) / numSteps);
  }

  return {};
}
} // namespace dynamic_paper
