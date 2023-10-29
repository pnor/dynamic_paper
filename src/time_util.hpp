#pragma once

#include <chrono>
#include <time.h>

#include "config.hpp"

/** Handling of time based events */

namespace dynamic_paper {

struct SunriseAndSunsetTimes {
  time_t sunrise;
  time_t sunset;
};

class SuneventPoller {
public:
  static std::string getSunriseAndSetString(const Config &config);
};

} // namespace dynamic_paper
