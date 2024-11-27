#include "location.hpp"

#include <exception>
#include <functional>
#include <string>

#include <boost/xpressive/xpressive_static.hpp>
#include <cpr/cpr.h>
#include <tl/expected.hpp>

#include "logger.hpp"

namespace dynamic_paper {

namespace {

constexpr std::string_view MOZILLA_LOCATION_URL =
    "https://location.services.mozilla.com/v1/geolocate?key=geoclue";
constexpr long SUCESS_CODE = 200;

tl::expected<double, LocationError>
tryParseDouble(const boost::xpressive::smatch &match) {
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

template <size_t NumberRetries>
cpr::Response getUrlWithRetry(const std::string_view urlString) {
  const cpr::Url url = cpr::Url{urlString};
  cpr::Response response{};
  size_t currentTry = 0;

  while (currentTry < NumberRetries) {
    response = cpr::Get(url);
    if (response.status_code == SUCESS_CODE) {
      break;
    }
    logWarning("Failed to get location with Mozilla: {} / {}", currentTry,
               NumberRetries);
    currentTry += 1;
  }

  return response;
}

} // namespace

// ===== Header ==========

tl::expected<std::pair<double, double>, LocationError>
getLatitudeAndLongitudeFromMozilla() {
  const cpr::Response response = getUrlWithRetry<3>(MOZILLA_LOCATION_URL);

  if (response.status_code != SUCESS_CODE) {
    return tl::unexpected(LocationError::RequestFailed);
  }

  const std::string &text = response.text;

  using namespace boost::xpressive;

  const boost::xpressive::sregex latitudeRegex =
      R"("lat: )" >> ('-' >> !+_d >> '.' >> *_d);
  boost::xpressive::smatch latitudeGroupMatches{};
  const bool foundLatitude =
      boost::xpressive::regex_search(text, latitudeGroupMatches, latitudeRegex);

  const boost::xpressive::sregex longitudeRegex =
      R"("lng: )" >> ('-' >> !+_d >> '.' >> *_d);
  boost::xpressive::smatch longitudeGroupMatches{};
  const bool foundLongitude = boost::xpressive::regex_search(
      text, longitudeGroupMatches, longitudeRegex);

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
