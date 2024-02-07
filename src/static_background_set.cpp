#include "static_background_set.hpp"

#include <cstdlib>

#include "background_setter.hpp"

namespace dynamic_paper {

// ===== Header ===============

namespace _helper {

/** Returns a random number in range [0..max)*/
size_t randomNumber(const size_t max) {
  std::random_device randomDevice;
  std::mt19937 gen(randomDevice());
  std::uniform_int_distribution<> distrib(0, static_cast<int>(max) - 1);
  return distrib(gen);
}

} // namespace _helper

StaticBackgroundData::StaticBackgroundData(std::filesystem::path dataDirectory,
                                           BackgroundSetMode mode,
                                           std::vector<std::string> imageNames)
    : dataDirectory(std::move(dataDirectory)), mode(mode),
      imageNames(std::move(imageNames)) {}

} // namespace dynamic_paper
