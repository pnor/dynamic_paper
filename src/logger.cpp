#include "logger.hpp"

#include <filesystem>
#include <iostream>
#include <string_view>

#include <spdlog/sinks/basic_file_sink.h>
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

void setupLogging(
    std::pair<LogLevel, std::filesystem::path> &&logLevelAndLogFile) {
  const std::pair<LogLevel, std::filesystem::path> levelAndFile =
      std::move(logLevelAndLogFile);

  const std::shared_ptr<spdlog::logger> console =
      spdlog::basic_logger_mt("main logger", std::string(levelAndFile.second));
  spdlog::set_default_logger(console);

  spdlog::set_pattern("[%H:%M:%S %z] [%^--%L--%$] %v");
  setShouldShowDebugLogs(levelAndFile.first);
}

} // namespace dynamic_paper
