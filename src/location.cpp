#include "location.hpp"

#include <exception>
#include <string>

#include <boost/xpressive/xpressive_static.hpp>
#include <cpr/cpr.h>
#include <tl/expected.hpp>

#include "logger.hpp"

namespace dynamic_paper {

namespace {

constexpr std::string_view LOCATION_URL = "https://ipapi.co/latlong/";
constexpr long SUCESS_CODE = 200;

template <size_t NumberRetries>
cpr::Response getUrlWithRetry(const std::string_view urlString) {
  const cpr::Url url = cpr::Url{urlString};
  cpr::Response response{};
  size_t currentTry = 0;

  while (currentTry < NumberRetries) {
    const int sleepTime =
        100 * (static_cast<int>(std::pow(2, currentTry) - 1.0F));
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));

    response = cpr::Get(url);
    if (response.status_code == SUCESS_CODE) {
      break;
    }
    logWarning("Failed to get location: {} / {}", currentTry, NumberRetries);
    currentTry += 1;
  }

  return response;
}

tl::expected<std::pair<double, double>, LocationError>
parseLatitudeAndLongitude(const std::string_view text) {
  // Expected response is formatted:
  // latitude,longitude
  //
  // example:
  // 37.347900,-121.852700

  const std::size_t commaLocation = text.find(',');
  if (commaLocation == std::string_view::npos) {
    logError("Unable to parse location: {}", text);
    return tl::make_unexpected(LocationError::UnableParseLatitudeOrLongitude);
  }

  const std::string_view latitudeString = text.substr(0, commaLocation);
  const std::string_view longitudeString = text.substr(commaLocation + 1);

  double latitude = NAN;
  double longitude = NAN;

  try {
    latitude = std::stod(std::string(latitudeString));
  } catch (const std::exception &exception) {
    logError("Error when parsing latitude from string {} due to exception: ${}",
             text, exception.what());
    return tl::make_unexpected(LocationError::UnableParseLatitudeOrLongitude);
  }

  try {
    longitude = std::stod(std::string(longitudeString));
  } catch (const std::exception &exception) {
    logError(
        "Error when parsing longitude from string {} due to exception: ${}",
        text, exception.what());
    return tl::make_unexpected(LocationError::UnableParseLatitudeOrLongitude);
  }

  return {std::make_pair(latitude, longitude)};
}

} // namespace

// ===== Header ==========

tl::expected<std::pair<double, double>, LocationError>
getLatitudeAndLongitudeFromHttp() {
  const cpr::Response response = getUrlWithRetry<3>(LOCATION_URL);

  if (response.status_code != SUCESS_CODE) {
    return tl::unexpected(LocationError::RequestFailed);
  }

  const std::string &text = response.text;

  return parseLatitudeAndLongitude(text);
}

} // namespace dynamic_paper
