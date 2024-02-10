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

/**
 * Type that can be used to create files and directories.
 * Used to allow a mock version to be used in testing to avoid creating real
 * files
 */
template <typename T>
concept ChangesFilesystem = requires(const std::filesystem::path &path,
                                     const std::string_view contents) {
  { T::createDirectoryIfDoesntExist(path) } -> std::convertible_to<bool>;
  { T::createFileIfDoesntExist(path, contents) } -> std::convertible_to<bool>;
  { T::exists(path) } -> std::convertible_to<bool>;
};

/** Class that has functions to change the filesystem */
class FilesystemHandler {
public:
  /** Creates directory called `dir`. Does nothing if it already exists. Returns
   * `true` if created the directory/it already exist, and `false` otherwise */
  static bool createDirectoryIfDoesntExist(const std::filesystem::path &dir);

  /** Creates file at `filePath` with `contents`. Does nothing if it already
   * exists. Will attempt to create parent directories if needed
   * Returns `true` if created the file/already existed, and `false` otherwise.
   */
  static bool createFileIfDoesntExist(const std::filesystem::path &filePath,
                                      std::string_view contents);

  /** Returns true if `path` exists */
  inline static bool exists(const std::filesystem::path &path) {
    return std::filesystem::exists(path);
  }
};

} // namespace dynamic_paper
