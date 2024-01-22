#include "file_util.hpp"

#include <fstream>

#include "logger.hpp"

namespace dynamic_paper {

// ===== Header ===============

bool createDirectoryIfDoesntExist(const std::filesystem::path &dir) {
  logTrace("Attempting to create directory {}", dir.string());

  if (std::filesystem::exists(dir)) {
    logInfo("Skipped creating {} because it already exists", dir.string());
    return true;
  }

  bool result = std::filesystem::create_directories(dir);

  if (result) {
    logInfo("Succesfully created the directory {}", dir.string());
  } else {
    logWarning("Unsuccesful in creating the directory {}", dir.string());
  }
  return result;
}

bool createFileIfDoesntExist(const std::filesystem::path &filePath,
                             const std::string_view contents) {
  logTrace("Attempting to create file {}", filePath.string());

  if (std::filesystem::exists(filePath)) {
    logInfo("Skipped creating {} because it already exists", filePath.string());
    return true;
  }

  if (filePath.has_parent_path()) {
    std::filesystem::create_directories(filePath.parent_path());
  }

  std::ofstream file(filePath);

  file << contents << std::flush;

  return true;
}

} // namespace dynamic_paper
