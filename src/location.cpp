#include "location.hpp"

#include <exception>
#include <functional>
#include <regex>
#include <string>

#include <cpr/cpr.h>
#include <tl/expected.hpp>

#include "logger.hpp"

namespace dynamic_paper {

namespace {

constexpr std::string_view MOZILLA_LOCATION_URL =
    "https://location.services.mozilla.com/v1/geolocate?key=geoclue";
constexpr long SUCESS_CODE = 200;

tl::expected<double, LocationError> tryParseDouble(const std::smatch &match) {
  return std::invoke([&match]() -> tl::expected<double, LocationError> {
    try {
      return std::stod(match[1]);
    } catch (const std::exception &exception) {
      logError("Error when parsing latitude/longitude location req: {}",
               exception.what());
      return tl::unexpected(LocationError::UnableParseLatitudeOrLongitude);
    }
  });
}

} // namespace

// ===== Header ==========

tl::expected<std::pair<double, double>, LocationError>
getLatitudeAndLongitudeFromMozilla() {
  const cpr::Response response = cpr::Get(cpr::Url{MOZILLA_LOCATION_URL});

  if (response.status_code != SUCESS_CODE) {
    return tl::unexpected(LocationError::RequestFailed);
  }

  const std::string &text = response.text;

  const std::regex latitudeRegex(R"("lat": (-?\d+\.\d*))");
  std::smatch latitudeGroupMatches{};
  const bool foundLatitude =
      std::regex_search(text, latitudeGroupMatches, latitudeRegex);

  const std::regex longitudeRegex(R"("lng": (-?\d+\.\d*))");
  std::smatch longitudeGroupMatches{};
  const bool foundLongitude =
      std::regex_search(text, longitudeGroupMatches, longitudeRegex);
  if (!foundLatitude || !foundLongitude) {
    return tl::unexpected(LocationError::UnableParseJsonResponse);
  }

  const tl::expected<double, LocationError> expectedLatitude =
      tryParseDouble(latitudeGroupMatches);
  const tl::expected<double, LocationError> expectedLongitude =
      tryParseDouble(longitudeGroupMatches);

  if (!expectedLatitude.has_value()) {
    return tl::unexpected(expectedLatitude.error());
  }
  if (!expectedLongitude.has_value()) {
    return tl::unexpected(expectedLongitude.error());
  }
  return {std::make_pair(expectedLatitude.value(), expectedLongitude.value())};
}

} // namespace dynamic_paper
