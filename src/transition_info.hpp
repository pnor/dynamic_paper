#pragma once

/**
 * Helper struct used to represent transitions
 */

#include <chrono>

#include "logger.hpp"

namespace dynamic_paper {

struct TransitionInfo {
  std::chrono::seconds duration;
  unsigned int steps;

  constexpr TransitionInfo(const std::chrono::seconds duration,
                           const unsigned int steps)
      : duration(duration), steps(steps) {
    logAssert(duration.count() > 0, "Transition duration must be > 0");
  }
};

} // namespace dynamic_paper
