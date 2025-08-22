#pragma once

/** Enums for different traits of `BackgroundSet`s*/

#include <string>
#include <cstdint>

#include "constants.hpp"
#include "logger.hpp"
#include "string_util.hpp"

namespace dynamic_paper {

enum class BackgroundSetMode: std::uint8_t { Center, Fill, Tile, Scale };

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

constexpr std::optional<BackgroundSetMode>
stringToBackgroundSetMode(const std::string_view modeString) {
  const std::string modeStringNormalized = normalize(std::string(modeString));

  if (modeStringNormalized == CENTER_STRING) {
    return BackgroundSetMode::Center;
  }
  if (modeStringNormalized == FILL_STRING) {
    return BackgroundSetMode::Fill;
  }
  if (modeStringNormalized == TILE_STRING) {
    return BackgroundSetMode::Tile;
  }
  if (modeStringNormalized == SCALE_STRING) {
    return BackgroundSetMode::Scale;
  }

  return std::nullopt;
}

enum class BackgroundSetOrder : std::uint8_t { Linear, Random };
enum class BackgroundSetType : std::uint8_t { Dynamic, Static };

} // namespace dynamic_paper
