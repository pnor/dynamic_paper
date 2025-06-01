#include "helper.hpp"

#include <filesystem>

#include "src/config.hpp"
#include "src/background_set_method.hpp"

dynamic_paper::Config getConfig() {
  return {testBackgroundSetConfigFile(), testHookScript, testImageCacheDir(),
          dynamic_paper::BackgroundSetMethod(dynamic_paper::MethodWallUtils{}),
          testSolarDayProvider()};
}

dynamic_paper::Config
getConfig(const std::filesystem::path &backgroundSetConfigPath) {
  return {backgroundSetConfigPath, testHookScript, testImageCacheDir(),
          dynamic_paper::BackgroundSetMethod(dynamic_paper::MethodWallUtils{}),
          testSolarDayProvider()};
}
