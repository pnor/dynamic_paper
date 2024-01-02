#pragma once

#include <cstddef>
#include <string_view>

namespace dynamic_paper {

/** Helper functions for logging */

/** To determine which log messages should be shown */
enum class LogLevel { INFO, WARNING, ERROR, DEBUG, CRITICAL, TRACE, OFF };

/** Sets up logging library, by setting the format and pattern of logs, and what
 * logs should be shown. */
void setupLogging(const LogLevel logLevel);

/** Prints a debug log message saying `msg`. */
void logDebug(const std::string_view msg);

/** Prints an informational log message saying `msg`. */
void logInfo(const std::string_view msg);

/** Prints a warning log message saying `msg`.*/
void logWarning(const std::string_view msg);

/** Prints an error log message saying `msg`. */
void logError(const std::string_view msg);

/** Prints a fatal error log message saying `msg`. */
void logFatalError(const std::string_view msg);

/** Asserts `condition`, printing a message if it fails and throwing an
 * exception. Otherwise, does nothing */
void logAssert(const bool condition, const std::string_view msg);

} // namespace dynamic_paper
