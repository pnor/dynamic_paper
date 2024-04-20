#pragma once

/**
 * Information related to how the general `Config` will determine the user's
 * location
 */

#include <utility>

namespace dynamic_paper {

/**
 * Describes how to infer the user's position
 *
 * If `useLocationInfoOverSearch` is true, will not try and search for the
 * user's position and use `latitudeAndLongitude` as the position.
 */
struct LocationInfo {
  std::pair<double, double> latitudeAndLongitude;
  /**
   * If true, will *not* search for the
   * user's position using a location service and instead use
   * `latitudeAndLongitude` as the position.
   */
  bool useLatitudeAndLongitudeOverLocationSearch;
};

} // namespace dynamic_paper
