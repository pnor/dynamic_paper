#pragma once

#include <iostream>

namespace dynamic_paper {

constexpr std::string_view ANSI_COLOR_RED = "\x1b[31m";
constexpr std::string_view ANSI_COLOR_GREEN = "\x1b[32m";
constexpr std::string_view ANSI_COLOR_YELLOW = "\x1b[33m";
constexpr std::string_view ANSI_COLOR_BLUE = "\x1b[34m";
constexpr std::string_view ANSI_COLOR_MAGENTA = "\x1b[35m";
constexpr std::string_view ANSI_COLOR_CYAN = "\x1b[36m";
constexpr std::string_view ANSI_COLOR_RESET = "\x1b[0m";

void inline logInfo(const std::string &msg, const bool flush = true) {
  if (flush) {
    std::cout << "[INFO] " << msg << std::endl;
  } else {
    std::cout << "[INFO] " << msg << std::endl;
  }
}

void inline logWarning(const std::string &msg, const bool flush = true) {
  if (flush) {
    std::cout << ANSI_COLOR_YELLOW << "[WARN] " << msg << ANSI_COLOR_RESET
              << std::endl;
  } else {
    std::cout << ANSI_COLOR_YELLOW << "[WARN] " << msg << "\n"
              << ANSI_COLOR_RESET;
  }
}

void inline logError(const std::string &msg, const bool flush = true) {
  if (flush) {
    std::cout << "[ERROR] " << msg << std::endl;
    std::cerr << "[ERROR] " << msg << std::endl;
  } else {
    std::cout << "[ERROR] " << msg << "\n";
    std::cerr << "[ERROR] " << msg << "\n";
  }
}

void inline logFatalError(const std::string &msg, const bool flush = true) {
  if (flush) {
    std::cout << ANSI_COLOR_RED << "[ERROR] " << msg << std::endl
              << ANSI_COLOR_RESET;
    std::cerr << "[ERROR] " << msg << std::endl;
  } else {
    std::cout << ANSI_COLOR_RED << "[ERROR] " << msg << "\n"
              << ANSI_COLOR_RESET;
    std::cerr << "[ERROR] " << msg << "\n";
  }
}

} // namespace dynamic_paper
