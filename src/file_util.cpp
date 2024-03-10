#include "file_util.hpp"

#include <fstream>
#include <ranges>

#include "logger.hpp"

namespace dynamic_paper {

// ===== Helper ======

namespace {

/** Creates `filePath` and fills it with `contents`, making directories on the
 * way to its path if they don't exist. */
bool createFile(const std::filesystem::path &filePath,
                const std::string_view contents) {
  if (std::filesystem::exists(filePath)) {
    return true;
  }

  if (filePath.has_parent_path()) {
    std::filesystem::create_directories(filePath.parent_path());
  }

  std::ofstream file(filePath);

  file << contents << std::flush;

  return true;
}

} // namespace

// ===== Header ===============

std::filesystem::path getHomeDirectory() {
  const char *envHomeDir = std::getenv("HOME");
  if (envHomeDir == nullptr) {
    return {"/root"};
  }
  return {std::string(envHomeDir)};
}

std::filesystem::path expandPath(std::filesystem::path path) {
  std::filesystem::path unexpandedPath = std::move(path);

  if (unexpandedPath.begin()->string() == "~") {
    std::filesystem::path newPath = getHomeDirectory();
    for (const auto &dir : unexpandedPath | std::views::drop(1)) {
      newPath /= dir;
    }
    return newPath;
  }

  return unexpandedPath;
}

bool FilesystemHandler::createDirectoryIfDoesntExist(
    const std::filesystem::path &dir) {

  if (std::filesystem::exists(dir)) {
    return true;
  }

  const bool result = std::filesystem::create_directories(dir);

  if (result) {
    logInfo("Succesfully created the directory {}", dir.string());
  } else {
    logWarning("Unsuccesful in creating the directory {}", dir.string());
  }
  return result;
}

bool FilesystemHandler::createFileIfDoesntExist(
    const std::filesystem::path &filePath, const std::string_view contents) {
  return createFile(expandPath(filePath), contents);
}

} // namespace dynamic_paper
