#pragma once

#include <string>
#include <tl/expected.hpp>

/*
** Making GET requests and storing it in a string using libcurl
*/

namespace dynamic_paper {

enum class NetworkError {
  BufferTooSmall,
  NetworkError,
  SystemError,
  RetryError,
  LogicError,
  UnknownError,
};

tl::expected<std::string, NetworkError> getFromURL(std::string_view url);

} // namespace dynamic_paper
