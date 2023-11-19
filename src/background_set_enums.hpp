#pragma once

#include <string>

#include "logger.hpp"

/** Enums for different traits of `BackgroundSet`s*/

namespace dynamic_paper {

enum class BackgroundSetMode { Center, Fill };

constexpr std::string backgroundSetModeString(const BackgroundSetMode mode) {
  switch (mode) {
  case BackgroundSetMode::Center: {
    return "center";
  }
  case BackgroundSetMode::Fill: {
    return "fill";
  }
  default: {
    logAssert(false, "Unable to convert background set mode to string");
  }
  }
}

enum class BackgroundSetOrder { Linear, Random };
enum class BackgroundSetType { Dynamic, Static };

} // namespace dynamic_paper
