#pragma once

#include <expected>
#include <filesystem>
#include <optional>
#include <type_traits>
#include <variant>

#include <yaml-cpp/yaml.h>

#include "background_set_enums.hpp"
#include "config.hpp"
#include "logger.hpp"
#include "type_helper.hpp"

/** Helper functions for parsing the YAML files*/

namespace dynamic_paper {

// ===== Constants ===============
constexpr std::string_view DYNAMIC_STRING = "dynamic";
constexpr std::string_view STATIC_STRING = "static";
constexpr std::string_view LINEAR_STRING = "linear";
constexpr std::string_view RANDOM_STRING = "random";
constexpr std::string_view CENTER_STRING = "center";
constexpr std::string_view FILL_STRING = "fill";
constexpr std::string_view SUNWAIT_STRING = "sunwait";
constexpr std::string_view SCRIPT_STRING = "script";
constexpr std::string_view WALLUTILS_STRING = "wallutils";

// ===== Error Types ===============
enum class BackgroundSetterMethodError {
  NoScriptProvided,
  NoMethodInYAML,
  InvalidMethod
};

// ===== Converting yaml strings to types ===============
// general template
template <typename T>
  requires(!is_optional<T>)
constexpr std::optional<T> stringTo(const std::string &s) {
  if constexpr (std::is_convertible_v<std::string,
                                      T>) { // trivial conversion from string
    return static_cast<T>(s);
  } else if constexpr (std::is_same_v<T, bool>) { // boolean
    if (s == "true") {
      return true;
    } else if (s == "false") {
      return false;
    } else {
      return std::nullopt;
    }
  } else if constexpr (std::is_unsigned_v<T> &&
                       std::is_integral_v<T>) { // unsigned
    T val = std::clamp(
        std::stoul(s),
        static_cast<long unsigned int>(std::numeric_limits<T>::min()),
        static_cast<long unsigned int>(std::numeric_limits<T>::max()));
    return static_cast<T>(val);
  } else if constexpr (std::is_signed_v<T> && std::is_integral_v<T>) { // signed
    T val = std::clamp(std::stol(s),
                       static_cast<long int>(std::numeric_limits<T>::min()),
                       static_cast<long int>(std::numeric_limits<T>::max()));
    return static_cast<T>(val);
  } else {
    return std::nullopt;
  }
}

// Matching string to enums
template <>
constexpr std::optional<SunEventPollerMethod> stringTo(const std::string &s) {
  if (s == SUNWAIT_STRING) {
    return std::make_optional(SunEventPollerMethod::Sunwait);
  } else {
    return std::nullopt;
  }
}

template <>
constexpr std::optional<BackgroundSetMode> stringTo(const std::string &s) {
  if (s == CENTER_STRING) {
    return std::make_optional(BackgroundSetMode::Center);
  } else if (s == FILL_STRING) {
    return std::make_optional(BackgroundSetMode::Fill);
  } else {
    return std::nullopt;
  }
}

template <>
constexpr std::optional<BackgroundSetOrder> stringTo(const std::string &s) {
  if (s == LINEAR_STRING) {
    return std::make_optional(BackgroundSetOrder::Linear);
  } else if (s == RANDOM_STRING) {
    return std::make_optional(BackgroundSetOrder::Random);
  } else {
    return std::nullopt;
  }
}

template <>
constexpr std::optional<BackgroundSetType> stringTo(const std::string &s) {
  if (s == DYNAMIC_STRING) {
    return std::make_optional(BackgroundSetType::Dynamic);
  } else if (s == STATIC_STRING) {
    return std::make_optional(BackgroundSetType::Static);
  } else {
    return std::nullopt;
  }
}

// ===== Parsing BackgroundSetterMethod Strings ===============
std::expected<BackgroundSetterMethod, BackgroundSetterMethodError>
parseBackgroundSetterMethod(YAML::Node config, const std::string &methodKey,
                            const std::string &scriptKey);

// ===== Parsing yaml ===============

template <typename T>
T generalConfigParseOrUseDefault(const YAML::Node &config,
                                 const std::string &key, const T defaultValue) {
  const YAML::Node node = config[key];

  if constexpr (is_optional<T>) {
    T nodeConverted = stringTo<typename T::value_type>(node.as<std::string>());
    T val = nodeConverted.has_value() ? nodeConverted.value() : defaultValue;
    return val;
  } else {
    T val = node.IsDefined()
                ? stringTo<T>(node.as<std::string>()).value_or(defaultValue)
                : defaultValue;
    return val;
  }
}

} // namespace dynamic_paper
