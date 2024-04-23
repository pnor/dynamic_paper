#include "helper.hpp"

#include <filesystem>

#include "src/config.hpp"

dynamic_paper::Config getConfig() {
  return {testBackgroundSetConfigFile(), testHookScript, testImageCacheDir(),
          testSolarDayProvider()};
}

dynamic_paper::Config
getConfig(const std::filesystem::path &backgroundSetConfigPath) {
  return {backgroundSetConfigPath, testHookScript, testImageCacheDir(),
          testSolarDayProvider()};
}
