#include "location.hpp"

#include <exception>
#include <string>

#include <boost/xpressive/xpressive_static.hpp>
#include <tl/expected.hpp>

#include "logger.hpp"
#include "networking.hpp"

namespace dynamic_paper {

namespace {

constexpr std::string_view LOCATION_URL = "https://ipapi.co/latlong/";
constexpr long SUCESS_CODE = 200;

using NetworkResponse = tl::expected<std::string, NetworkError>;

template <size_t NumberRetries>
NetworkResponse getUrlWithRetry(const std::string_view urlString) {
  NetworkResponse result = tl::make_unexpected(NetworkError::RetryError);

  size_t currentTry = 0;

  while (currentTry < NumberRetries) {
    const int sleepTime =
        100 * (static_cast<int>(std::pow(2, currentTry) - 1.0F));
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));

    result = getFromURL(urlString);
    if (result.has_value()) {
      return result;
    }
    logWarning("Failed to get location: {} / {}", currentTry, NumberRetries);
    currentTry += 1;
  }

  return result;
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
  const NetworkResponse response = getUrlWithRetry<3>(LOCATION_URL);

  if (!response.has_value()) {
    logError("Failed to get location using a network request to {}",
             LOCATION_URL);
    return tl::unexpected(LocationError::RequestFailed);
  }

  const std::string &text = response.value();

  return parseLatitudeAndLongitude(text);
}

} // namespace dynamic_paper
