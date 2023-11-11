#pragma once

#include <iostream>

namespace dynamic_paper {

void inline logInfo(const std::string &msg, const bool flush = true) {
  if (flush) {
    std::cout << "[INFO] " << msg << std::endl;
  } else {
    std::cout << "[INFO] " << msg << std::endl;
  }
}

void inline logWarning(const std::string &msg, const bool flush = true) {
  if (flush) {
    std::cout << "[WARN] " << msg << std::endl;
  } else {
    std::cout << "[WARN] " << msg << "\n";
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

} // namespace dynamic_paper
