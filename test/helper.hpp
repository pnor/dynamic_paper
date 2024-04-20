/**
 * Functions used in multiple places across tests
 */

#include "src/config.hpp"
#include "src/defaults.hpp"
#include "src/solar_day_provider.hpp"

inline std::filesystem::path testBackgroundSetConfigFile() {
  return {"./test_default_background_set_config.yaml"};
}

const std::optional<std::filesystem::path> testHookScript = std::nullopt;

inline std::filesystem::path testImageCacheDir() { return {"./test_bg_cache"}; }

inline dynamic_paper::SolarDayProvider testSolarDayProvider() {
  return {dynamic_paper::ConfigDefaults::solarDay};
}

dynamic_paper::Config getConfig();

dynamic_paper::Config
getConfig(const std::filesystem::path &backgroundSetConfigPath);
