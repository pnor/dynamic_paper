#pragma once

#include <string>
#include <tl/expected.hpp>
#include  <cstdint>

/*
** Making GET requests and storing it in a string using libcurl
*/

namespace dynamic_paper {

enum class NetworkError: std::uint8_t {
  BufferTooSmall,
  NetworkError,
  SystemError,
  RetryError,
  LogicError,
  UnknownError,
};

tl::expected<std::string, NetworkError> getFromURL(const std::string& url);

} // namespace dynamic_paper
