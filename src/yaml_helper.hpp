#pragma once

#include <optional>

#include <yaml-cpp/yaml.h>

#include "config.hpp"

/** Helper functions for parsing the YAML files*/

namespace dynamic_paper {

template <typename T>
static std::optional<T> stringToType(const std::string &s) {
  return std::make_optional(static_cast<T>(s));
}

template <>
std::optional<BackgroundSetterMethod> stringToType(const std::string &s) {
  if (s == "script") {
    return std::make_optional(BackgroundSetterMethod::Script);
  } else if (s == "wallutils") {
    return std::make_optional(BackgroundSetterMethod::WallUtils);
  } else {
    return std::nullopt;
  }
}

template <>
std::optional<std::filesystem::path> stringToType(const std::string &s) {
  return std::make_optional(std::filesystem::path(s));
}

template <typename T>
T parseOrUseDefault(const YAML::Node &config, const std::string &key,
                    const T defaultValue) {
  YAML::Node node = config[key];
  T val = node.IsDefined()
              ? stringToType<T>(node.as<std::string>()).value_or(defaultValue)
              : defaultValue;
  return val;
}

} // namespace dynamic_paper
