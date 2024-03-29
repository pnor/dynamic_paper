#pragma once

/**
 * Use mozilla location services to estimate the location of the user.
 * Used to determine when sunrise and sunset is
 */

#include <utility>

#include "config.hpp"

namespace dynamic_paper {

std::pair<double, double> getUserLatitudeAndLongitude(const Config &config);

}
