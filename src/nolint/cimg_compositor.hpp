#pragma once

#include <filesystem>

namespace dynamic_paper {

/**
 * Create and save a merged image using `startImagePath` and `endImagePath`
 * using CImg. Saves to `destinationPath` and composites inline such a way that
 * `percentage`% of the startImagePath is included in the output.
 */
void compositeUsingCImg(const std::filesystem::path &startImagePath,
                        const std::filesystem::path &endImagePath,
                        const std::filesystem::path &destinationImagePath,
                        unsigned int percentage);

} // namespace dynamic_paper
