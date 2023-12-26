#include "static_background_set.hpp"

#include <cstdlib>
#include <format>
#include <random>

#include "background_setter.hpp"
#include "config.hpp"
#include "logger.hpp"
#include "variant_visitor_templ.hpp"

namespace dynamic_paper {

static void printDebugInfo(const StaticBackgroundData *data,
                           const std::string &imageName,
                           const BackgroundSetterMethod &method) {
  std::string modeString = backgroundSetModeString(data->mode);
  std::string methodString = backgroundSetterMethodString(method);
  logWarning(std::format(
      "Encountered error in attempting to set the background for static "
      "background set. Image name was {} with mode {} using method {}",
      imageName, modeString, methodString));
}

/** Returns a random number in range [0..max)*/
static int randomNumber(const int max) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(0, max - 1);
  return distrib(gen);
}

StaticBackgroundData::StaticBackgroundData(std::filesystem::path dataDirectory,
                                           BackgroundSetMode mode,
                                           std::vector<std::string> imageNames)
    : dataDirectory(dataDirectory), mode(mode), imageNames(imageNames) {}

void StaticBackgroundData::show(const Config &config) const {
  logInfo("Showing static background");
  logAssert(imageNames.size() > 0, "Static background");

  const std::string imageName = imageNames[randomNumber(imageName.size())];
  const std::filesystem::path imagePath = dataDirectory / imageName;

  std::expected<void, BackgroundError> result =
      setBackgroundToImage(imagePath, mode, config.backgroundSetterMethod);

  if (!result.has_value()) {
    printDebugInfo(this, imageName, config.backgroundSetterMethod);
  }

  if (config.hookScript.has_value()) {
    runHookCommand(config.hookScript.value(), imagePath);
  }
}

} // namespace dynamic_paper
