#pragma once

#include <filesystem>
#include <string>

#include <tl/expected.hpp>

namespace dynamic_paper {

/** Logic of creating and caching composited backgrounds */

enum class CompositeImageError { UnableToCreatePath, CommandError };

/**
 * Returns the path to an image that is interpolated between
 * `commonImageDirectory /startImageName` and `commonImageDirectory /
 * endImageName` by `percentage`%. Uses the external program `imagemagick` to
 * create the interpolated image
 *
 * `percentage` should be in the range [0..100]
 */
tl::expected<std::filesystem::path, CompositeImageError> getCompositedImage(
    const std::filesystem::path &commonImageDirectory,
    const std::string &startImageName, const std::string &endImageName,
    const std::filesystem::path &cacheDirectory, unsigned int percentage);
} // namespace dynamic_paper
