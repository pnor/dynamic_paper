#include "logger.hpp"

#include <iostream>
#include <string_view>

#include <spdlog/spdlog.h>

namespace dynamic_paper {

static void setShouldShowDebugLogs(const LogLevel logLevel) {
  switch (logLevel) {
  case LogLevel::DEBUG: {
    spdlog::set_level(spdlog::level::debug);
    break;
  }
  case LogLevel::INFO: {
    spdlog::set_level(spdlog::level::info);
    break;
  }
  case LogLevel::WARNING: {
    spdlog::set_level(spdlog::level::warn);
    break;
  }
  case LogLevel::ERROR: {
    spdlog::set_level(spdlog::level::err);
    break;
  }
  case LogLevel::CRITICAL: {
    spdlog::set_level(spdlog::level::critical);
    break;
  }
  case LogLevel::TRACE: {
    spdlog::set_level(spdlog::level::trace);
    break;
  }
  case LogLevel::OFF: {
    spdlog::set_level(spdlog::level::off);
    break;
  }
  }
}

// ===== Header ===============

void setupLogging(const LogLevel logLevel) {
  spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] %v");
  setShouldShowDebugLogs(logLevel);
}

void logInfo(const std::string_view msg) { spdlog::info(msg); }

void logDebug(const std::string_view msg) { spdlog::debug(msg); }

void logWarning(const std::string_view msg) { spdlog::warn(msg); }

void logError(const std::string_view msg) { spdlog::error(msg); }

void logFatalError(const std::string_view msg) { spdlog::critical(msg); }

void logAssert(const bool condition, const std::string_view msg) {
  if (condition) {
    return;
  }

  spdlog::critical(msg);

  throw std::logic_error("Assertion failed: " + std::string(msg));
}

} // namespace dynamic_paper
