#include "image_compositor.hpp"

#include <format>

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
    logWarning(std::format("{} and {} are not the same type of image!",
                           startImageName, endImageName));
  }
  if (extension == "") {
    logWarning(std::format("will use {} for the file extension but it does not "
                           "have a filetype extension!",
                           startImageName));
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
    logWarning(std::format(
        "Trying to make a composite image using {} but it doesn't exist!",
        startImagePath.string()));
  }
  if (!std::filesystem::exists(endImagePath)) {
    logWarning(std::format(
        "Trying to make a composite image using {} but it doesn't exist!",
        endImagePath.string()));
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

std::expected<std::filesystem::path, CompositeImageError>
getCompositedImage(const std::filesystem::path &commonImageDirectory,
                   const std::string &startImageName,
                   const std::string &endImageName,
                   const unsigned int percentage, const Config &config) {
  logAssert(percentage <= 100, "percentage must be in range [0..100] but was " +
                                   std::to_string(percentage));

  if (percentage == 0) {
    return commonImageDirectory / startImageName;
  }
  if (percentage == 100) {
    return commonImageDirectory / endImageName;
  }

  const std::expected<std::filesystem::path, CompositeImageError>
      compositeImagePath =
          pathForCompositeImage(commonImageDirectory, startImageName,
                                endImageName, percentage,
                                config.imageCacheDirectory);

  if (!compositeImagePath.has_value()) {
    return std::unexpected(expCompositeImagePath.error());
  }

  if (std::filesystem::exists(compositeImagePath.value())) {
    return compositeImagePath.value();
  }

  return createCompositeImage(commonImageDirectory / startImageName,
                              commonImageDirectory / endImageName,
                              compositeImagePath.value(), percentage);

  // TODO
  // *** create the unique path for the interpolated image:
  // - (remove the background image dir)
  // - combine dataDir and imageName into a string
  // - (example: [place]-[image])
  //
  // combine 2 image paths name to get unique name for interpolation from
  // + add percentage to name
  // (example: [place1]-[image1]-[place2]-[image2]-[percentage].jpg)
  //
  // *** startImage to endImage try and get it from the cache dir
  // ~/.cache/dynamic_paper/[unique_name]
  //
  // *** if it exists, return it (return [unique name])
  //
  // *** if it doesn't, create it:
  // run `magick -gravity center -blend [percentage]% [path to image1] [path to
  // image 2] [unqiue name]`
  //
  // *** return [unique name]

  // TODO does this work if u merge a png and jpg?
  // should alert the user if filetypes don't match ...
}

} // namespace dynamic_paper
