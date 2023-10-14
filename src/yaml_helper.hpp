#pragma once

#include <optional>
#include <type_traits>

#include <yaml-cpp/yaml.h>

#include "background_set_enums.hpp"
#include "config.hpp"

/** Helper functions for parsing the YAML files*/

namespace dynamic_paper {

// ===== Converting yaml strings to types ===============
// general template
template <typename T>
constexpr std::optional<T> stringTo(const std::string &s) {
  if constexpr (std::is_convertible_v<std::string, T>) {
    return std::make_optional(static_cast<T>(s));
  } else {
    return std::nullopt;
  }
}

// Matching string to enums
template <>
constexpr std::optional<BackgroundSetterMethod> stringTo(const std::string &s) {
  if (s == "script") {
    return std::make_optional(BackgroundSetterMethod::Script);
  } else if (s == "wallutils") {
    return std::make_optional(BackgroundSetterMethod::WallUtils);
  } else {
    return std::nullopt;
  }
}

template <>
constexpr std::optional<BackgroundSetMode> stringTo(const std::string &s) {
  if (s == "center") {
    return std::make_optional(BackgroundSetMode::Center);
  } else if (s == "fill") {
    return std::make_optional(BackgroundSetMode::Fill);
  } else {
    return std::nullopt;
  }
}

template <>
constexpr std::optional<BackgroundSetOrder> stringTo(const std::string &s) {
  if (s == "linear") {
    return std::make_optional(BackgroundSetOrder::Linear);
  } else if (s == "random") {
    return std::make_optional(BackgroundSetOrder::Random);
  } else {
    return std::nullopt;
  }
}

template <>
constexpr std::optional<BackgroundSetType> stringTo(const std::string &s) {
  if (s == "dynamic") {
    return std::make_optional(BackgroundSetType::Dynamic);
  } else if (s == "static") {
    return std::make_optional(BackgroundSetType::Static);
  } else {
    return std::nullopt;
  }
}

// ===== Parsing yaml ===============

template <typename T>
T generalConfigParseOrUseDefault(const YAML::Node &config,
                                 const std::string &key, const T defaultValue) {
  YAML::Node node = config[key];
  T val = node.IsDefined()
              ? stringTo<T>(node.as<std::string>()).value_or(defaultValue)
              : defaultValue;
  return val;
}

} // namespace dynamic_paper
