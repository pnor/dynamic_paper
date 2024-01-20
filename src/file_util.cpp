#include "file_util.hpp"

#include <fstream>

#include "logger.hpp"

namespace dynamic_paper {

// ===== Helper ===============
namespace {

bool createParentDirectories(const std::filesystem::path &filePath) {
  if (std::filesystem::exists(filePath)) {
    return true;
  } else {
    logTrace("Creating parent directory: {}", filePath.string());

    if (filePath.has_parent_path()) {
      return createParentDirectories(filePath.parent_path()) &&
             createDirectoryIfDoesntExist(filePath);
    } else {
      return createDirectoryIfDoesntExist(filePath);
    }
  }
}

} // namespace

// ===== Header ===============

bool createDirectoryIfDoesntExist(const std::filesystem::path &dir) {
  logTrace("Attempting to create directory {}", dir.string());

  if (std::filesystem::exists(dir)) {
    logInfo("Skipped creating {} because it already exists", dir.string());
    return true;
  }

  bool result = std::filesystem::create_directory(dir);

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
    createParentDirectories(filePath.parent_path());
  }

  std::ofstream file(filePath);

  file << contents << std::flush;

  return true;
}

} // namespace dynamic_paper
