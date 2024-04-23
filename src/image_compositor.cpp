#include "image_compositor.hpp"

#include <format>

#include "logger.hpp"
#include "nolint/cimg_compositor.hpp"

namespace dynamic_paper {

// ===== Helper ===============

namespace {

constexpr unsigned int EMPTY_PERCENT = 0;
constexpr unsigned int MAX_PERCENT = 100;

std::optional<std::string> basename(const std::filesystem::path &path) {
  if (std::distance(path.begin(), path.end()) == 0) {
    return std::nullopt;
  }

  std::string basename;
  std::filesystem::path::iterator pathIterator = path.end();
  while (basename.empty()) {
    basename = *(--pathIterator);
  }
  return basename;
}

bool filesHaveSameExtension(const std::string &name1,
                            const std::string &name2) {
  return std::filesystem::path(name1).extension() ==
         std::filesystem::path(name2).extension();
}

std::string getExtension(const std::string &startName,
                         const std::string &endName) {
  const std::filesystem::path startPath(startName);
  if (startPath.extension().empty()) {
    return std::filesystem::path(endName).extension();
  }
  return startPath.extension();
}

tl::expected<std::filesystem::path, CompositeImageError>
createCompositeImage(const std::filesystem::path &startImagePath,
                     const std::filesystem::path &endImagePath,
                     const std::filesystem::path &destinationImagePath,
                     const unsigned int percentage) {
  if (!std::filesystem::exists(startImagePath)) {
    logWarning(
        "Trying to make a composite image using {} but it doesn't exist!",
        startImagePath.string());
    return tl::unexpected(CompositeImageError::FileDoesntExist);
  }

  if (!std::filesystem::exists(endImagePath)) {
    logWarning(
        "Trying to make a composite image using {} but it doesn't exist!",
        endImagePath.string());
    return tl::unexpected(CompositeImageError::FileDoesntExist);
  }
  if (std::filesystem::exists(destinationImagePath)) {
    logWarning("Creating a new composite image that already exists in cache!");
  }

  compositeUsingCImg(startImagePath, endImagePath, destinationImagePath,
                     percentage);

  return destinationImagePath;
}

} // namespace

// ===== Header ===============

tl::expected<std::filesystem::path, CompositeImageError>
pathForCompositeImage(const std::filesystem::path &commonImageDirectory,
                      const std::string &startImageName,
                      const std::string &endImageName,
                      const unsigned int percentage,
                      const std::filesystem::path &cacheDirectory) {
  const std::optional<std::string> optDirName = basename(commonImageDirectory);
  if (!optDirName.has_value()) {
    return tl::unexpected(CompositeImageError::UnableToCreatePath);
  }

  const std::string &dirName = optDirName.value();
  const std::string extension = getExtension(startImageName, endImageName);

  if (!filesHaveSameExtension(startImageName, endImageName)) {
    logWarning("{} and {} are not the same type of image!", startImageName,
               endImageName);
  }
  if (extension.empty()) {
    logWarning("will use '{}' for the file extension but it does not "
               "have a filetype extension!",
               startImageName);
  }

  const std::string startStem = std::filesystem::path(startImageName).stem();
  const std::string endStem = std::filesystem::path(endImageName).stem();

  const std::string compositeName = std::format(
      "{}-{}-{}-{}{}", dirName, startStem, endStem, percentage, extension);

  return cacheDirectory / compositeName;
}

tl::expected<std::filesystem::path, CompositeImageError>
ImageCompositor::getCompositedImage(
    const std::filesystem::path &commonImageDirectory,
    const std::string &startImageName, const std::string &endImageName,
    const std::filesystem::path &cacheDirectory,
    const unsigned int percentage) {
  logAssert(percentage <= MAX_PERCENT,
            "percentage must be in range [0..100] but was {}", percentage);

  if (percentage == EMPTY_PERCENT) {
    return commonImageDirectory / startImageName;
  }
  if (percentage == MAX_PERCENT) {
    return commonImageDirectory / endImageName;
  }

  const tl::expected<std::filesystem::path, CompositeImageError>
      compositeImagePath =
          pathForCompositeImage(commonImageDirectory, startImageName,
                                endImageName, percentage, cacheDirectory);

  if (!compositeImagePath.has_value()) {
    return tl::unexpected(compositeImagePath.error());
  }

  if (std::filesystem::exists(compositeImagePath.value())) {
    return compositeImagePath.value();
  }

  return createCompositeImage(commonImageDirectory / startImageName,
                              commonImageDirectory / endImageName,
                              compositeImagePath.value(), percentage);
}

} // namespace dynamic_paper
