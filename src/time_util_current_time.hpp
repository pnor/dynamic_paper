#pragma once

#include "time_from_midnight.hpp"

namespace dynamic_paper {

/**
 * Returns the current time using the system clock, as seconds from midnight
 */
TimeFromMidnight getCurrentTime();

} // namespace dynamic_paper
