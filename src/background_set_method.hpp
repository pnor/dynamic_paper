#pragma once

#include <variant>
#include <filesystem>

/** Methods to set the background */

namespace dynamic_paper {

struct MethodWallUtils {};

using BackgroundSetMethod = std::variant<MethodWallUtils, std::filesystem::path>;

} // namespace dynamic_paper
