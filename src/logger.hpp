#pragma once

/** Helper functions for logging */

#include <filesystem>

#include <spdlog/spdlog.h>

#include "format.hpp"

namespace dynamic_paper {

/** To determine which log messages should be shown */
enum class LogLevel { INFO, WARNING, ERROR, DEBUG, CRITICAL, TRACE, OFF };

/**
 * Flush the logger on all levels
 */
void flushLogger();

/** Sets up logging library, by setting the format and pattern of logs, what
 * logs should be shown, and where to log to */
void setupLogging(
    std::pair<LogLevel, std::filesystem::path> &&logLevelAndLogFile);

/**
 * Sets up logging library to print to stdout, and no logfile
 */
void setupLoggingForStdout(LogLevel level);

/** Prints a debug log message saying `msg`. Accepts `args` for use in a format
 * string. */
template <typename... Ts>
void logDebug(const FormatString<Ts...> &msg, Ts &&...args) {
  spdlog::debug(dynamic_paper::format<Ts...>(msg, std::forward<Ts>(args)...));
}

/** Prints an informational log message saying `msg`. Accepts `args` for use in
 * a format string. */
template <typename... Ts>
void logInfo(const FormatString<Ts...> &msg, Ts &&...args) {
  spdlog::info(dynamic_paper::format<Ts...>(msg, std::forward<Ts>(args)...));
}

/** Prints a trace log message saying `msg`. Accepts `args` for use in a format
 * string. */
template <typename... Ts>
constexpr void logTrace(const FormatString<Ts...> &msg, Ts &&...args) {
  spdlog::trace(dynamic_paper::format<Ts...>(msg, std::forward<Ts>(args)...));
}

/** Prints a warning log message saying `msg`. Accepts `args` for use in a
 * format string.*/
template <typename... Ts>
constexpr void logWarning(const FormatString<Ts...> &msg, Ts &&...args) {
  spdlog::warn(dynamic_paper::format<Ts...>(msg, std::forward<Ts>(args)...));
}

/** Prints an error log message saying `msg`. Accepts `args` for use in a format
 * string. */
template <typename... Ts>
constexpr void logError(const FormatString<Ts...> &msg, Ts &&...args) {
  spdlog::error(dynamic_paper::format<Ts...>(msg, std::forward<Ts>(args)...));
}

/** Prints a fatal error log message saying `msg`. Accepts `args` for use in a
 * format string. */
template <typename... Ts>
constexpr void logFatalError(const FormatString<Ts...> &msg, Ts &&...args) {
  spdlog::critical(
      dynamic_paper::format<Ts...>(msg, std::forward<Ts>(args)...));
}

/** Asserts `condition`, printing a message if it fails and throwing an
 * exception. Otherwise, does nothing.
 * Accepts `args` for use in a format string.
 */
template <typename... Ts>
constexpr void logAssert(const bool condition, const FormatString<Ts...> &msg,
                         Ts &&...args) {
  if (condition) {
    return;
  }

  spdlog::critical(dynamic_paper::format(msg, std::forward<Ts>(args)...));

  throw std::logic_error(
      "Assertion failed: " +
      dynamic_paper::format<Ts...>(msg, std::forward<Ts>(args)...));
}

} // namespace dynamic_paper
