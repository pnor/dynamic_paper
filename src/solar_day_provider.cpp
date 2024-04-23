#include "solar_day_provider.hpp"

#include "location_info.hpp"
#include "solar_day.hpp"
#include "time_util.hpp"
#include "variant_visitor_templ.hpp"

namespace dynamic_paper {

SolarDay SolarDayProvider::getSolarDay() const {
  return std::visit(overloaded{
                        [](const LocationInfo &locationInfo) {
                          return getSolarDayUsingLocation(locationInfo);
                        },
                        [](const SolarDay &solarDay) { return solarDay; },
                    },
                    locationOrDefaultDay);
}

SolarDayProvider::SolarDayProvider(LocationInfo locationInfo)
    : locationOrDefaultDay(std::move(locationInfo)) {}

SolarDayProvider::SolarDayProvider(const SolarDay &solarDay)
    : locationOrDefaultDay(solarDay) {}

} // namespace dynamic_paper
