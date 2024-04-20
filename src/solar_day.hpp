#pragma once

/** Time of the sunrise and sunset in one abstraction */

#include "time_from_midnight.hpp"

namespace dynamic_paper {

/** Storage class for sunrise and sunset times */
struct SolarDay {
  TimeFromMidnight sunrise;
  TimeFromMidnight sunset;

  constexpr bool operator==(const SolarDay &) const noexcept = default;
};

} // namespace dynamic_paper
