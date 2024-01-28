#pragma once

#include <filesystem>
#include <optional>

/** Utility functions to work with the filesystem */

namespace dynamic_paper {

/** Returns the path to the user's home directory, as described by the
 * environement variable $HOME. If $HOME is unset, or is unable to get a valid
 * value from $HOME, will default to root's home, `/root` */
std::filesystem::path getHomeDirectory();

/** Expands `path` so that a leading "~" is replaced with the user's home
 * directory. */
std::filesystem::path expandPath(std::filesystem::path path);

/** Creates directory called `dir`. Does nothing if it already exists. Returns
 * `true` if created the directory/it already exist, and `false` otherwise */
bool createDirectoryIfDoesntExist(const std::filesystem::path &dir);

/** Creates file at `filePath` with `contents`. Does nothing if it already
 * exists. Will attempt to create parent directories if needed
 * Returns `true` if created the file/already existed, and `false` otherwise. */
bool createFileIfDoesntExist(const std::filesystem::path &filePath,
                             std::string_view contents);

} // namespace dynamic_paper
