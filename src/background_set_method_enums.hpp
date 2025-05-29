#pragma once

#include <cstdint>

/** Enums for methods to set the background */

namespace dynamic_paper {

enum class BackgroundSetMethod : std::uint8_t {
  WallUtils,
  Script,
};

}
