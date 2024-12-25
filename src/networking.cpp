#include "networking.hpp"

#include <array>
#include <cstring>

#include <curl/curl.h>
#include <tl/expected.hpp>

#include "logger.hpp"

namespace dynamic_paper {

namespace {

// 23 is the exact number of bytes needed to store a resposnse formatted
// latitude,longitude
constexpr std::size_t CURL_BUFFER_SIZE = 23;

template <std::size_t N>
size_t writeChunk(void *ptr, size_t size, size_t nmemb,
                  std::string *stringPtr) {
  if ((size * nmemb) + 1 > N) {
    return 0;
  }

  std::array<char, N> newData{0};
  std::memcpy(newData.begin(), ptr, size * nmemb);

  stringPtr->append(static_cast<char *>(newData.begin()));

  return size * nmemb;
}

tl::expected<std::string, NetworkError>
handleResponseCode(std::string &&responsePayload, CURLcode responseCode,
                   const std::string_view url) {
  std::string payload = std::move(responsePayload);

  switch (responseCode) {
  case CURLE_OK: {
    logInfo("Successfully made network request to {} and got back {}", url,
            payload);
    return {std::move(payload)};
  }
  case CURLE_WRITE_ERROR: {
    logError("Buffer of size {} bytes was too small to receive chunks of the "
             "resposne from {}",
             CURL_BUFFER_SIZE, url);
    return tl::make_unexpected(NetworkError::BufferTooSmall);
  }
  case CURLE_COULDNT_RESOLVE_HOST: {
    logError("Could not resolve host when trying to connect to {}", url);
    return tl::make_unexpected(NetworkError::NetworkError);
  }
  case CURLE_HTTP_RETURNED_ERROR: {
    logError("HTTP returned error when trying to connect to {}", url);
    return tl::make_unexpected(NetworkError::NetworkError);
  }
  case CURLE_OUT_OF_MEMORY: {
    logError("Encountered an Out of Memory error when trying to connect to {}",
             url);
    return tl::make_unexpected(NetworkError::SystemError);
  }
  case CURLE_OPERATION_TIMEDOUT: {
    logError("Operation timed out when trying to connect to {}", url);
    return tl::make_unexpected(NetworkError::NetworkError);
  }
  case CURLE_TOO_MANY_REDIRECTS: {
    logError("Too many redirects when trying to connect to {}", url);
    return tl::make_unexpected(NetworkError::NetworkError);
  }
  case CURLE_SETOPT_OPTION_SYNTAX: {
    logError("Bad setopt option provided to libcurl");
    return tl::make_unexpected(NetworkError::LogicError);
  }
  case CURLE_RECV_ERROR: {
    logError("Failed to send network data when trying to connect to {}", url);
    return tl::make_unexpected(NetworkError::NetworkError);
  }
  case CURLE_LOGIN_DENIED: {
    logError("Failed to login to {}", url);
    return tl::make_unexpected(NetworkError::NetworkError);
  }
  case CURLE_AGAIN: {
    logError("Socket was not ready to recv/send, try again to connect to {}.",
             url);
    return tl::make_unexpected(NetworkError::NetworkError);
  }
  case CURLE_CHUNK_FAILED: {
    logError("Chunk callback reported error when trying to connect to {}.",
             url);
    return tl::make_unexpected(NetworkError::LogicError);
  }
  case CURLE_AUTH_ERROR: {
    logError(
        "Authentication function returned an error whe trying to connect to {}",
        url);
    return tl::make_unexpected(NetworkError::NetworkError);
  }
  default: {
    logError("Failed due to an unknown error when connecting to {}", url);
    return tl::make_unexpected(NetworkError::UnknownError);
  }
  }
}

} // namespace

// ===== Header ==============

tl::expected<std::string, NetworkError> getFromURL(const std::string_view url) {
  std::string responsePayload{};

  CURL *curl = nullptr;
  CURLcode responseCode{};

  curl = curl_easy_init();

  if (curl != nullptr) {
    // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
    curl_easy_setopt(curl, CURLOPT_URL, url.data());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeChunk<CURL_BUFFER_SIZE>);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responsePayload);
    // NOLINTEND(cppcoreguidelines-pro-type-vararg)
    responseCode = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
  }

  return handleResponseCode(std::move(responsePayload), responseCode, url);
}

} // namespace dynamic_paper
