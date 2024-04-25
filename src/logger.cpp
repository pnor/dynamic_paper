#include "logger.hpp"

#include <csignal>
#include <filesystem>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace dynamic_paper {

namespace {

constexpr std::string GLOBAL_LOGGER_NAME = "main logger";

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

void flushLogger(int /*unused*/) { spdlog::get(GLOBAL_LOGGER_NAME)->flush(); }

void setupFlushingOnSIGINT() {
  struct sigaction sigIntHandler = {};

  sigIntHandler.sa_handler = flushLogger;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, nullptr);
}

} // namespace

// ===== Header ===============

void flushLogger() { spdlog::get(GLOBAL_LOGGER_NAME)->flush(); }

void setupLogging(
    std::pair<LogLevel, std::filesystem::path> &&logLevelAndLogFile) {
  const std::pair<LogLevel, std::filesystem::path> levelAndFile =
      std::move(logLevelAndLogFile);

  const std::shared_ptr<spdlog::logger> console = spdlog::basic_logger_mt(
      GLOBAL_LOGGER_NAME, std::string(levelAndFile.second));
  spdlog::set_default_logger(console);

  spdlog::set_pattern("[%H:%M:%S %z] [%^--%L--%$] %v");
  setShouldShowDebugLogs(levelAndFile.first);

  setupFlushingOnSIGINT();
}

void setupLoggingForStdout(LogLevel level) {
  const std::shared_ptr<spdlog::logger> console =
      spdlog::stdout_color_mt("main logger");
  spdlog::set_default_logger(console);

  spdlog::set_pattern("[%H:%M:%S %z] [%^--%L--%$] %v");
  setShouldShowDebugLogs(level);
}

} // namespace dynamic_paper
