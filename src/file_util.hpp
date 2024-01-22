#pragma once

#include <filesystem>

/** Utility functions to work with the filesystem */

namespace dynamic_paper {

/** Creates directory called `dir`. Does nothing if it already exists. Returns
 * `true` if created the directory/it already exist, and `false` otherwise */
bool createDirectoryIfDoesntExist(const std::filesystem::path &dir);

/** Creates file at `filePath` with `contents`. Does nothing if it already
 * exists. Will attempt to create parent directories if needed
 * Returns `true` if created the file/already existed, and `false` otherwise. */
bool createFileIfDoesntExist(const std::filesystem::path &filePath,
                             std::string_view contents);

} // namespace dynamic_paper
