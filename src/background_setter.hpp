#pragma once

#include <filesystem>
#include <string>

#include <tl/expected.hpp>

#include "background_set_enums.hpp"
#include "config.hpp"
#include "file_util.hpp"
#include "image_compositor.hpp"
#include "transition_info.hpp"

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
    requires(T &&func, const std::filesystem::path &imagePath,
             const BackgroundSetMode mode,
             const BackgroundSetterMethod &method) {
      {
        func(imagePath, mode, method)
      } -> std::convertible_to<tl::expected<void, BackgroundError>>;
    };

tl::expected<void, BackgroundError>
setBackgroundToImage(const std::filesystem::path &imagePath,
                     BackgroundSetMode mode,
                     const BackgroundSetterMethod &method);

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
template <CanSetBackgroundTrait T, ChangesFilesystem Files = FilesystemHandler,
          GetsCompositeImages CompositeImages = ImageCompositor>
tl::expected<void, BackgroundError> lerpBackgroundBetweenImages(
    const std::filesystem::path &commonImageDirectory,
    const std::string &beforeImageName, const std::string &afterImageName,
    const std::filesystem::path &cacheDirectory, std::chrono::seconds duration,
    unsigned int numSteps, BackgroundSetMode mode,
    const BackgroundSetterMethod &method, T backgroundSetFunction);

/**
 * Runs the script at `hookScriptPath` on the image at `imagePath`.
 *
 * Used to run user defined hooks after the background changes.
 */
tl::expected<void, HookCommandError>
runHookCommand(const std::filesystem::path &hookScriptPath,
               const std::filesystem::path &imagePath);

// ===== Definition ==========

template <CanSetBackgroundTrait T, ChangesFilesystem Files,
          GetsCompositeImages CompositeImages>
tl::expected<void, BackgroundError> lerpBackgroundBetweenImages(
    const std::filesystem::path &commonImageDirectory,
    const std::string &beforeImageName, const std::string &afterImageName,
    const std::filesystem::path &cacheDirectory,
    const TransitionInfo &transition, const BackgroundSetMode mode,
    const BackgroundSetterMethod &method, T backgroundSetFunction) {

  const bool dirCreationResult =
      Files::createDirectoryIfDoesntExist(cacheDirectory);
  if (!dirCreationResult) {
    return tl::make_unexpected(BackgroundError::NoCacheDir);
  }

  for (unsigned int i = 0; i < transition.steps; i++) {

    // modifies `i` so is more in the middle of the range, to avoid
    // interpolating 0% and 100% images
    const unsigned int numerator = i + 1;
    const unsigned int denominator = transition.steps + 1;

    const float percentageFloat =
        static_cast<float>(numerator) / static_cast<float>(denominator);
    const unsigned int percentage = std::clamp(
        static_cast<unsigned int>(percentageFloat * 100.0), 0U, 100U);

    const tl::expected<std::filesystem::path, CompositeImageError>
        expectedCompositedImage =
            CompositeImages::getCompositedImage(commonImageDirectory,
                                                beforeImageName, afterImageName,
                                                cacheDirectory, percentage);

    if (!expectedCompositedImage.has_value()) {
      return tl::unexpected(BackgroundError::CompositeImageError);
    }

    const tl::expected<void, BackgroundError> backgroundResult =
        backgroundSetFunction(expectedCompositedImage.value(), mode, method);

    logTrace("Interpolating to {}...",
             expectedCompositedImage.value().string());

    if (!backgroundResult.has_value()) {
      return tl::unexpected(BackgroundError::SetBackgroundError);
    }

    // TODO should check `CompositeImages` is not a testing class instead of
    // checking exactly `ImageCompositor`
    if constexpr (std::is_same_v<CompositeImages, ImageCompositor>) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(transition.duration) / transition.steps);
    }
  }

  return {};
}
} // namespace dynamic_paper
