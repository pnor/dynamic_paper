#include "logger.hpp"

#include <iostream>
#include <string_view>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace dynamic_paper {

namespace {

void setShouldShowDebugLogs(const LogLevel logLevel) {
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

} // namespace

// ===== Header ===============

void setLoggingToStderr() {
  const std::shared_ptr<spdlog::logger> console =
      spdlog::stderr_color_mt("main logger");
  spdlog::set_default_logger(console);
}

void setupLogging(const LogLevel logLevel) {
  spdlog::set_pattern("[%H:%M:%S %z] [%^--%L--%$] %v");
  setShouldShowDebugLogs(logLevel);
}

} // namespace dynamic_paper
