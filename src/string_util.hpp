#pragma once

#include <algorithm>
#include <string>

/** Utility functions to operate on strings */

// === string manip ===

// trim from start (in place)
inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
}

// trim from end (in place)
inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

// trim from both ends (in place)
inline void trim(std::string &s) {
  rtrim(s);
  ltrim(s);
}

// trim from both ends (copying)
inline std::string trim_copy(std::string s) {
  trim(s);
  return s;
}

/**
 * "Normalized" a string such that it has no leading or trailing spaces and is
 * all lowercase. More for use in config parsing
 *
 * Example:
 * "  Dynamic " => "dynamic"
 * */
static constexpr std::string normalize(const std::string &s) {
  std::string sCopy = trim_copy(s);
  std::transform(sCopy.begin(), sCopy.end(), sCopy.begin(),
                 [](const char c) { return tolower(c); });
  return sCopy;
}
