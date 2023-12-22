#pragma once

#include <expected>
#include <filesystem>

#include "config.hpp"
#include "static_background_set.hpp"

/** Handles the logic of how the bckground is actually changed */

namespace dynamic_paper {

enum class BackgroundError { CommandError };

/**
 * Using a program specified by `method`, sets the background to the image in
 * `imagePath` using a display mode of `mode`
 */
std::expected<void, BackgroundError>
setBackgroundToImage(const std::filesystem::path &imagePath,
                     const BackgroundSetMode mode,
                     const BackgroundSetterMethod &method);

/**
 * Using a program specified by `method`, changes the background from
 * `beforePath` to `afterPath`. This effect occurs for `duration` seconds and
 * happens in `numSteps` steps. Sets the background using a program specified by
 * `method`, with a display mode of `mode`.
 *
 * Uses the external program imagemagick to create interpolated images.
 */
std::expected<void, BackgroundError> lerpBackgroundBetweenImages(
    const std::filesystem::path &beforePath,
    const std::filesystem::path &afterPath, const unsigned int duration,
    const unsigned int numSteps, const BackgroundSetMode mode,
    const BackgroundSetterMethod &method);
} // namespace dynamic_paper
