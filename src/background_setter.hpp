#pragma once

#include <filesystem>

#include <tl/expected.hpp>

#include "config.hpp"
#include "static_background_set.hpp"

/** Handles the logic of how the bckground is actually changed */

namespace dynamic_paper {

enum class BackgroundError {
  CommandError,
  CompositeImageError,
  SetBackgroundError,
  NoCacheDir,
};
enum class HookCommandError { CommandError };

/**
 * Using a program specified by `method`, sets the background to the image in
 * `imagePath` using a display mode of `mode`
 */
tl::expected<void, BackgroundError>
setBackgroundToImage(const std::filesystem::path &imagePath,
                     const BackgroundSetMode mode,
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
tl::expected<void, BackgroundError> lerpBackgroundBetweenImages(
    const std::filesystem::path &commonImageDirectory,
    const std::string &beforeImageName, const std::string &afterImageName,
    const std::filesystem::path &cacheDirectory,
    const std::chrono::seconds duration, const unsigned int numSteps,
    const BackgroundSetMode mode, const BackgroundSetterMethod &method);

/**
 * Runs the script at `hookScriptPath` on the image at `imagePath`.
 *
 * Used to run user defined hooks after the background changes.
 */
tl::expected<void, HookCommandError>
runHookCommand(const std::filesystem::path &hookScriptPath,
               const std::filesystem::path &imagePath);

} // namespace dynamic_paper
