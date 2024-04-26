#pragma once

/** Enums for different traits of `BackgroundSet`s*/

#include <string>

#include "logger.hpp"

namespace dynamic_paper {

enum class BackgroundSetMode { Center, Fill, Tile, Scale };

constexpr std::string backgroundSetModeString(const BackgroundSetMode mode) {
  switch (mode) {
  case BackgroundSetMode::Center: {
    return "center";
  }
  case BackgroundSetMode::Fill: {
    return "fill";
  }
  case BackgroundSetMode::Tile: {
    return "tile";
  }
  case BackgroundSetMode::Scale: {
    return "scale";
  }
  }

  logAssert(false, "Unable to construct mode sring from passed mode");
  return "";
}

enum class BackgroundSetOrder { Linear, Random };
enum class BackgroundSetType { Dynamic, Static };

} // namespace dynamic_paper
