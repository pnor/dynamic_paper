#pragma once

/**
 * How the general config determines when the Solar Day is
 */

#include "location_info.hpp"
#include "solar_day.hpp"

namespace dynamic_paper {

/**
 * How `Config` determines the Solar Day.
 */
class SolarDayProvider {
public:
  [[nodiscard]] SolarDay getSolarDay() const;

  SolarDayProvider(LocationInfo locationInfo);
  SolarDayProvider(const SolarDay &solarDay);

private:
  std::variant<LocationInfo, SolarDay> locationOrDefaultDay;
};
} // namespace dynamic_paper
