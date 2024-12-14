#pragma once

/**
 * Use mozilla location services to estimate the location of the user.
 * Used to determine when sunrise and sunset is
 */

#include <utility>

#include <tl/expected.hpp>

namespace dynamic_paper {

enum class LocationError {
  RequestFailed,
  UnableParseJsonResponse,
  UnableParseLatitudeOrLongitude,
};

/**
 * Gets the user's location using mozilla location
 *
 * Returns pair of {latitude, longitude}
 */
tl::expected<std::pair<double, double>, LocationError>
getLatitudeAndLongitudeFromHttp();

} // namespace dynamic_paper
