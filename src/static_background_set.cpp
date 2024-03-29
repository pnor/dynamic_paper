#include "static_background_set.hpp"

#include <cstdlib>
#include <random>

namespace dynamic_paper {

// ===== Header ===============

namespace detail {

/** Returns a random number in range [0..max)*/
size_t randomNumber(const size_t max) {
  std::random_device randomDevice;
  std::mt19937 gen(randomDevice());
  std::uniform_int_distribution<> distrib(0, static_cast<int>(max) - 1);
  return distrib(gen);
}

} // namespace detail

StaticBackgroundData::StaticBackgroundData(std::filesystem::path dataDirectory,
                                           BackgroundSetMode mode,
                                           std::vector<std::string> imageNames)
    : dataDirectory(std::move(dataDirectory)), mode(mode),
      imageNames(std::move(imageNames)) {}

} // namespace dynamic_paper
