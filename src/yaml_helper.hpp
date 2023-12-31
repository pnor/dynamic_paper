#pragma once

#include <algorithm>
#include <expected>
#include <filesystem>
#include <optional>
#include <type_traits>
#include <variant>

#include <yaml-cpp/yaml.h>

#include "background_set_enums.hpp"
#include "config.hpp"
#include "logger.hpp"
#include "string_util.hpp"
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

constexpr std::string_view INFO_LOGGING_STRING = "info";
constexpr std::string_view WARNING_LOGGING_STRING = "warning";
constexpr std::string_view ERROR_LOGGING_STRING = "error";
constexpr std::string_view DEBUG_LOGGING_STRING = "debug";
constexpr std::string_view CRITICAL_LOGGING_STRING = "critical";
constexpr std::string_view TRACE_LOGGING_STRING = "trace";
constexpr std::string_view OFF_LOGGING_STRING = "off";

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
static constexpr std::optional<T> stringTo(const std::string &s) {
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

// Specialization Matching string to enums
template <>
constexpr std::optional<SunEventPollerMethod> stringTo(const std::string &s) {
  const std::string configString = normalize(s);

  if (configString == SUNWAIT_STRING) {
    return std::make_optional(SunEventPollerMethod::Sunwait);
  } else {
    return std::nullopt;
  }
}

template <>
constexpr std::optional<BackgroundSetMode> stringTo(const std::string &s) {
  const std::string configString = normalize(s);

  if (configString == CENTER_STRING) {
    return std::make_optional(BackgroundSetMode::Center);
  } else if (configString == FILL_STRING) {
    return std::make_optional(BackgroundSetMode::Fill);
  } else {
    return std::nullopt;
  }
}

template <>
constexpr std::optional<BackgroundSetOrder> stringTo(const std::string &s) {
  const std::string configString = normalize(s);

  if (configString == LINEAR_STRING) {
    return std::make_optional(BackgroundSetOrder::Linear);
  } else if (configString == RANDOM_STRING) {
    return std::make_optional(BackgroundSetOrder::Random);
  } else {
    return std::nullopt;
  }
}

template <>
constexpr std::optional<BackgroundSetType> stringTo(const std::string &s) {
  const std::string configString = normalize(s);

  if (configString == DYNAMIC_STRING) {
    return std::make_optional(BackgroundSetType::Dynamic);
  } else if (configString == STATIC_STRING) {
    return std::make_optional(BackgroundSetType::Static);
  } else {
    return std::nullopt;
  }
}

template <> constexpr std::optional<LogLevel> stringTo(const std::string &s) {
  const std::string configString = normalize(s);

  if (configString == INFO_LOGGING_STRING) {
    return std::make_optional(LogLevel::INFO);
  }

  if (configString == WARNING_LOGGING_STRING) {
    return std::make_optional(LogLevel::WARNING);
  }

  if (configString == ERROR_LOGGING_STRING) {
    return std::make_optional(LogLevel::ERROR);
  }

  if (configString == DEBUG_LOGGING_STRING) {
    return std::make_optional(LogLevel::DEBUG);
  }

  if (configString == CRITICAL_LOGGING_STRING) {
    return std::make_optional(LogLevel::CRITICAL);
  }

  if (configString == TRACE_LOGGING_STRING) {
    return std::make_optional(LogLevel::TRACE);
  }

  if (configString == OFF_LOGGING_STRING) {
    return std::make_optional(LogLevel::OFF);
  }

  return std::nullopt;
}

// ===== Parsing BackgroundSetterMethod Strings ===============

/**
 *  Attempt to parse the Method of setting the background from the YAML.
 *
 *  If the yaml specifies the method to be 'script', will also read the
 * 'method_script' field to determine the script to use to set the background.
 *  If no method script is specified returns an unexpected error.
 *
 *  If no method is specified, defaults to 'Wallutils'
 */
std::expected<BackgroundSetterMethod, BackgroundSetterMethodError>
parseBackgroundSetterMethod(YAML::Node config, const std::string_view methodKey,
                            const std::string_view criptKey);

// ===== Parsing yaml ===============

template <typename T>
T generalConfigParseOrUseDefault(const YAML::Node &config,
                                 const std::string_view key,
                                 const T defaultValue) {
  const YAML::Node node = config[key];

  if constexpr (is_optional<T>) {
    if (!node.IsDefined()) {
      return std::nullopt;
    }

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
