#pragma once

#include <algorithm>
#include <regex>
#include <string>

/** Utility functions to operate on strings */

// === string manip ===

// trim from start (in place)
constexpr void ltrim(std::string &text) {
  text.erase(text.begin(),
             std::find_if(text.begin(), text.end(), [](unsigned char letter) {
               return std::isspace(letter) == 0;
             }));
}

// trim from end (in place)
constexpr void rtrim(std::string &text) {
  text.erase(std::find_if(
                 text.rbegin(), text.rend(),
                 [](unsigned char letter) { return std::isspace(letter) == 0; })
                 .base(),
             text.end());
}

// trim from both ends (in place)
constexpr void trim(std::string &text) {
  rtrim(text);
  ltrim(text);
}

// trim from both ends (copying)
constexpr std::string trim_copy(std::string text) {
  trim(text);
  return text;
}

/**
 * "Normalized" a string such that it has no leading or trailing spaces and is
 * all lowercase. More for use in config parsing
 *
 * Example:
 * "  Dynamic " => "dynamic"
 * */
constexpr std::string normalize(const std::string &text) {
  std::string sCopy = trim_copy(text);
  std::transform(sCopy.begin(), sCopy.end(), sCopy.begin(),
                 [](const char letter) { return tolower(letter); });
  return sCopy;
}

/**
 * Returns `true` if `text` matches any of `regex` or `otherRegexes`, and
 * `false` otherwise. Stores regex match information in `groupMatches`.
 */
template <typename... Ts>
inline bool tryRegexes(const std::string &text, std::smatch &groupMatches,
                       const std::regex &regex, Ts... otherRegexes) {
  if constexpr (sizeof...(Ts) == 0) {
    return std::regex_match(text, groupMatches, regex);
  } else {
    return std::regex_match(text, groupMatches, regex) ||
           tryRegexes(text, groupMatches, otherRegexes...);
  }
}
