#pragma once

/** Helper functions for logging */

#include <filesystem>
#include <format>

#include <spdlog/spdlog.h>

namespace dynamic_paper {

/** To determine which log messages should be shown */
enum class LogLevel { INFO, WARNING, ERROR, DEBUG, CRITICAL, TRACE, OFF };

/** Sets up logging library, by setting the format and pattern of logs, what
 * logs should be shown, and where to log to */
void setupLogging(
    std::pair<LogLevel, std::filesystem::path> &&logLevelAndLogFile);

/** Prints a debug log message saying `msg`. Accepts `args` for use in a format
 * string. */
template <typename... Ts>
void logDebug(const std::format_string<Ts...> &msg, Ts &&...args) {
  spdlog::debug(std::format(msg, std::forward<Ts>(args)...));
}

/** Prints an informational log message saying `msg`. Accepts `args` for use in
 * a format string. */
template <typename... Ts>
void logInfo(const std::format_string<Ts...> &msg, Ts &&...args) {
  spdlog::info(std::format(msg, std::forward<Ts>(args)...));
}

/** Prints a trace log message saying `msg`. Accepts `args` for use in a format
 * string. */
template <typename... Ts>
constexpr void logTrace(const std::format_string<Ts...> &msg, Ts &&...args) {
  spdlog::trace(std::format(msg, std::forward<Ts>(args)...));
}

/** Prints a warning log message saying `msg`. Accepts `args` for use in a
 * format string.*/
template <typename... Ts>
constexpr void logWarning(const std::format_string<Ts...> &msg, Ts &&...args) {
  spdlog::warn(std::format(msg, std::forward<Ts>(args)...));
}

/** Prints an error log message saying `msg`. Accepts `args` for use in a format
 * string. */
template <typename... Ts>
constexpr void logError(const std::format_string<Ts...> &msg, Ts &&...args) {
  spdlog::error(std::format(msg, std::forward<Ts>(args)...));
}

/** Prints a fatal error log message saying `msg`. Accepts `args` for use in a
 * format string. */
template <typename... Ts>
constexpr void logFatalError(const std::format_string<Ts...> &msg,
                             Ts &&...args) {
  spdlog::critical(std::format(msg, std::forward<Ts>(args)...));
}

/** Asserts `condition`, printing a message if it fails and throwing an
 * exception. Otherwise, does nothing.
 * Accepts `args` for use in a format string.
 */
template <typename... Ts>
constexpr void logAssert(const bool condition,
                         const std::format_string<Ts...> &msg, Ts &&...args) {
  if (condition) {
    return;
  }

  spdlog::critical(std::format(msg, std::forward<Ts>(args)...));

  throw std::logic_error("Assertion failed: " +
                         std::format(msg, std::forward<Ts>(args)...));
}

} // namespace dynamic_paper
