#pragma once

/**
 * How the program executes the user provided hook script
 */

#include <filesystem>

#include <tl/expected.hpp>

namespace dynamic_paper {

enum class HookError { ForkError };

/**
 * Executes the hook script pointed to by `hookScriptPath`, passing the image
 * name, `imagePath` as an arguement.
 */
[[nodiscard]] tl::expected<void, HookError>
runHookScript(const std::filesystem::path &hookScriptPath,
              const std::filesystem::path &imagePath);

} // namespace dynamic_paper
