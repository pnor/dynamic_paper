#pragma once

#include <filesystem>

namespace dynamic_paper {

/**
 * Create and save a merged image using `startImagePath` and `endImagePath`
 * using Image Magick. Saves to `destinationPath` and composites in such a way
 * that `percentage`% of the startImagePath is included in the output.
 */
void compositeUsingImageMagick(
    const std::filesystem::path &startImagePath,
    const std::filesystem::path &endImagePath,
    const std::filesystem::path &destinationImagePath, unsigned int percentage);

} // namespace dynamic_paper
