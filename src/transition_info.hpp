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

  /**
   * Whether the transition will create new files or edit one in place
   */
  bool inPlace;

  constexpr TransitionInfo(const std::chrono::seconds duration,
                           const unsigned int steps, const bool inPlace)
      : duration(duration), steps(steps), inPlace(inPlace) {
    logAssert(duration.count() > 0, "Transition duration must be > 0");
  }
};

} // namespace dynamic_paper
