#pragma once

#include <expected>
#include <filesystem>

#include "config.hpp"
#include "static_background_set.hpp"

/** Handles the logic of how the bckground is actually changed */

namespace dynamic_paper {

enum class BackgroundError { CommandError, NoHookScriptFound };

std::expected<bool, BackgroundError>
setBackgroundToImage(const std::filesystem::path &imagePath,
                     const BackgroundSetMode mode,
                     const BackgroundSetterMethod method);

} // namespace dynamic_paper
