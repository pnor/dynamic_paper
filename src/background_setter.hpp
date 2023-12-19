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

std::expected<void, BackgroundError>
lerpBackgroundBetweenImages(const std::filesystem::path &beforePath,
                            const std::filesystem::path &afterPath,
                            const BackgroundSetMode mode,
                            const BackgroundSetterMethod &method);
} // namespace dynamic_paper
