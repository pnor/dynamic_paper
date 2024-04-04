#pragma once

/**
 * Use mozilla location services to estimate the location of the user.
 * Used to determine when sunrise and sunset is
 */

#include <utility>

#include <tl/expected.hpp>

#include "config.hpp"

namespace dynamic_paper {

enum class LocationError {
  RequestFailed,
  UnableParseJsonResponse,
  UnableParseLatitudeOrLongitude,
};

/**
 * Gets the user's location
 *
 * Returns pair of {latitude, longitude}
 */
tl::expected<std::pair<double, double>, LocationError>
getUserLatitudeAndLongitude(const Config &config);

} // namespace dynamic_paper
