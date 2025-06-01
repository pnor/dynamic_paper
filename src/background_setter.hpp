#pragma once

/** Handles the logic of how the bckground is actually changed */

#include <filesystem>
#include <string>
#include <cstdint>

#include <tl/expected.hpp>

#include "background_set_enums.hpp"
#include "file_util.hpp"
#include "image_compositor.hpp"

namespace dynamic_paper {

/** Errors that can occur when setting the background */
enum class BackgroundError : std::uint8_t {
  CommandError,
  CompositeImageError,
  NoCacheDir,
};
/** Errors that occur when running a hook command */
enum class HookCommandError : std::uint8_t { CommandError };

/**
 * Trait for a type that contains a static function that can change the
 * background. Use as an abstraction to allow testing without changing global
 * state
 */
template <typename T>
concept CanSetBackgroundTrait =
    requires(T &&func, const std::filesystem::path &imagePath,
             const BackgroundSetMode mode) {
      { func(imagePath, mode) } -> std::convertible_to<void>;
    };

/**
 * Changes the background from `commonImageDirectory / beforeImageName` to
 * `commonImageDirectory / afterImageName`. This effect occurs for `duration`
 * seconds and happens in `numSteps` steps. Sets the background using a program
 * specified by `method`, with a display mode of `mode`.
 * Uses `cacheDirectory` to store and get composited images.
 */
template <CanSetBackgroundTrait T, ChangesFilesystem Files = FilesystemHandler,
          GetsCompositeImages CompositeImages = ImageCompositor>
tl::expected<void, BackgroundError> lerpBackgroundBetweenImages(
    const std::filesystem::path &commonImageDirectory,
    const std::string &beforeImageName, const std::string &afterImageName,
    const std::filesystem::path &cacheDirectory, std::chrono::seconds duration,
    unsigned int numSteps, BackgroundSetMode mode, T backgroundSetFunction);

/**
 * Return a function that sets the background using `scriptPath`.
 * Intended to be then used as the `CanSetBackgroundTrait` argument in other
 * functions to set the background using a script
 */
template <CanSetBackgroundTrait T>
T setBackgroundToImageUsingScriptLambda(const std::filesystem::path &scriptPath,
                                        const std::filesystem::path &imagePath,
                                        const BackgroundSetMode mode) {
  return [scriptPath](const std::filesystem::path &imagePath,
                      const BackgroundSetMode mode) {

  };
}

void setBackgroundToImage(const std::filesystem::path &imagePath,
                          BackgroundSetMode mode);

void setBackgroundToImageUsingScript(const std::filesystem::path &scriptPath,
                                     const std::filesystem::path &imagePath,
                                     BackgroundSetMode mode);

} // namespace dynamic_paper
