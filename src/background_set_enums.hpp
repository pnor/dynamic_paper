#pragma once

#include <string>

#include "logger.hpp"

/** Enums for different traits of `BackgroundSet`s*/

namespace dynamic_paper {

// TODO include stretch tile and scale
enum class BackgroundSetMode { Center, Fill };

constexpr std::string backgroundSetModeString(const BackgroundSetMode mode) {
  switch (mode) {
  case BackgroundSetMode::Center: {
    return "center";
  }
  case BackgroundSetMode::Fill: {
    return "fill";
  }
  }

  logAssert(false, "Unable to construct mode sring from passed mode");
  return "";
}

enum class BackgroundSetOrder { Linear, Random };
enum class BackgroundSetType { Dynamic, Static };

} // namespace dynamic_paper
