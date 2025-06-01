#pragma once

/**
 * How the program executes the user provided hook script
 */

#include <filesystem>
#include <cstdint>

#include <tl/expected.hpp>

#include "background_set_enums.hpp"

namespace dynamic_paper {

enum class ScriptError: std::uint8_t { ForkError };

/**
 * Executes the hook script pointed to by `scriptPath`, passing the image
 * name, `imagePath` as an arguement.
 */
[[nodiscard]] tl::expected<void, ScriptError>
runHookScript(const std::filesystem::path &scriptPath,
              const std::filesystem::path &imagePath);
/**
 * Executes the script pointed to by `scriptPath`, to set the background image.
 * Passes in `imagePath` and `method` as arguments to the script
 */
[[nodiscard]] tl::expected<void, ScriptError>
runBackgroundSetScript(const std::filesystem::path &scriptPath,
              const std::filesystem::path &imagePath,
              BackgroundSetMode mode);

} // namespace dynamic_paper
