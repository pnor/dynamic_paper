#include "image_compositor.hpp"

#include <format>

#include "command_executor.hpp"
#include "logger.hpp"

namespace dynamic_paper {

// ===== Helper ===============

static std::optional<std::string> basename(const std::filesystem::path &p) {
  if (std::distance(p.begin(), p.end()) == 0) {
    return std::nullopt;
  }

  std::string basename;
  std::filesystem::path::iterator pathIterator = p.end();
  while (basename == "") {
    basename = *(--pathIterator);
  }
  return basename;
}

static bool filesHaveSameExtension(const std::string &name1,
                                   const std::string &name2) {
  return std::filesystem::path(name1).extension() ==
         std::filesystem::path(name2).extension();
}

static std::expected<std::filesystem::path, CompositeImageError>
pathForCompositeImage(const std::filesystem::path &commonImageDirectory,
                      const std::string &startImageName,
                      const std::string &endImageName,
                      const unsigned int percentage,
                      const std::filesystem::path &cacheDirectory) {
  const std::optional<std::string> optDirName = basename(commonImageDirectory);
  if (!optDirName.has_value()) {
    return std::unexpected(CompositeImageError::UnableToCreatePath);
  }

  const std::string &dirName = optDirName.value();
  const std::string extension =
      std::filesystem::path(startImageName).extension();

  if (!filesHaveSameExtension(startImageName, endImageName)) {
    logWarning("{} and {} are not the same type of image!", startImageName,
               endImageName);
  }
  if (extension == "") {
    logWarning("will use {} for the file extension but it does not "
               "have a filetype extension!",
               startImageName);
  }

  const std::string compositeName =
      std::format("{}-{}-{}-{}{}", dirName, startImageName, endImageName,
                  percentage, extension);

  return cacheDirectory / compositeName;
}

static std::expected<std::filesystem::path, CompositeImageError>
createCompositeImage(const std::filesystem::path &startImagePath,
                     const std::filesystem::path &endImagePath,
                     const std::filesystem::path &destinationImagePath,
                     const unsigned int percentage) {
  if (!std::filesystem::exists(startImagePath)) {
    logWarning(
        "Trying to make a composite image using {} but it doesn't exist!",
        startImagePath.string());
  }
  if (!std::filesystem::exists(endImagePath)) {
    logWarning(
        "Trying to make a composite image using {} but it doesn't exist!",
        endImagePath.string());
  }
  if (std::filesystem::exists(destinationImagePath)) {
    logWarning("Creating a new composite image that already exists in cache!");
  }

  const int exitCode = runCommandExitCode(
      std::format("magick composite -gravity center -blend {}% {} {} {}",
                  percentage, startImagePath.string(), endImagePath.string(),
                  destinationImagePath.string()));

  if (exitCode == EXIT_SUCCESS) {
    return destinationImagePath;
  } else {
    return std::unexpected(CompositeImageError::CommandError);
  }
}

// ===== Header ===============

std::expected<std::filesystem::path, CompositeImageError>
getCompositedImage(const std::filesystem::path &commonImageDirectory,
                   const std::string &startImageName,
                   const std::string &endImageName,
                   const std::filesystem::path &cacheDirectory,
                   const unsigned int percentage) {
  logAssert(percentage <= 100,
            "percentage must be in range [0..100] but was {}", percentage);

  if (percentage == 0) {
    return commonImageDirectory / startImageName;
  }
  if (percentage == 100) {
    return commonImageDirectory / endImageName;
  }

  const std::expected<std::filesystem::path, CompositeImageError>
      compositeImagePath =
          pathForCompositeImage(commonImageDirectory, startImageName,
                                endImageName, percentage, cacheDirectory);

  if (!compositeImagePath.has_value()) {
    return std::unexpected(compositeImagePath.error());
  }

  if (std::filesystem::exists(compositeImagePath.value())) {
    return compositeImagePath.value();
  }

  return createCompositeImage(commonImageDirectory / startImageName,
                              commonImageDirectory / endImageName,
                              compositeImagePath.value(), percentage);
}

} // namespace dynamic_paper
