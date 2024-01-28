#include "static_background_set.hpp"

#include <cstdlib>
#include <format>
#include <random>

#include <tl/expected.hpp>
#include <utility>

#include "background_setter.hpp"
#include "config.hpp"
#include "logger.hpp"
#include "variant_visitor_templ.hpp"

namespace dynamic_paper {

namespace {

void printDebugInfo(const StaticBackgroundData *data,
                    const std::string &imageName,
                    const BackgroundSetterMethod &method) {
  std::string modeString = backgroundSetModeString(data->mode);
  std::string methodString = backgroundSetterMethodString(method);
  logWarning("Encountered error in attempting to set the background for static "
             "background set. Image name was {} with mode {} using method {}",
             imageName, modeString, methodString);
}

/** Returns a random number in range [0..max)*/
size_t randomNumber(const size_t max) {
  std::random_device randomDevice;
  std::mt19937 gen(randomDevice());
  std::uniform_int_distribution<> distrib(0, static_cast<int>(max) - 1);
  return distrib(gen);
}

} // namespace

// ===== Header ===============

StaticBackgroundData::StaticBackgroundData(std::filesystem::path dataDirectory,
                                           BackgroundSetMode mode,
                                           std::vector<std::string> imageNames)
    : dataDirectory(std::move(dataDirectory)), mode(mode),
      imageNames(std::move(imageNames)) {}

void StaticBackgroundData::show(const Config &config) const {
  logTrace("Showing static background");
  logAssert(imageNames.empty(), "Static background cannot show with no images");

  const std::string imageName = imageNames.at(randomNumber(imageNames.size()));
  const std::filesystem::path imagePath = dataDirectory / imageName;

  const tl::expected<void, BackgroundError> result =
      setBackgroundToImage(imagePath, mode, config.backgroundSetterMethod);

  if (!result.has_value()) {
    printDebugInfo(this, imageName, config.backgroundSetterMethod);
  }

  if (config.hookScript.has_value()) {
    runHookCommand(config.hookScript.value(), imagePath);
  }
}

} // namespace dynamic_paper
