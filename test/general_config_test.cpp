/**
 *   Test parsing of YAML config files
 */

#include <gtest/gtest.h>

#include <yaml-cpp/yaml.h>

#include "src/config.hpp"

using namespace dynamic_paper;

namespace {

constexpr std::string_view DEFAULT_BACKGROUND_SET_CONFIG =
    "~/.local/share/dynamic_paper/background_sets.yaml";
constexpr std::string_view DEFAULT_CACHE_IMAGE_DIR = "~/.cache/dynamic_paper";

constexpr std::string_view GENERAL_CONFIG_YAML = R""""(
method: "wallutils"
sun_poller: "sunwait"
background_config: "./an_image_dir"
hook_script: "./hook_script.sh"
cache_dir: "~/.cache/backgrounds"
)"""";

constexpr std::string EMPTY_YAML;

constexpr std::string_view GENERAL_CONFIG_ONLY_METHOD_YAML = R""""(
method: "wallutils"
)"""";

constexpr std::string_view METHOD_WALLUTILS = R""""(
method: "wallutils"
sun_poller: "sunwait"
background_config: "./image"
)"""";

constexpr std::string_view METHOD_SCRIPT = R""""(
method: "script"
method_script: "./script.sh"
sun_poller: "sunwait"
background_config: "./image"
)"""";

constexpr std::string_view BOTH_METHOD_AND_SCRIPT = R""""(
method: "wallutils"
method_script: "./script.sh"
sun_poller: "sunwait"
background_config: "./image"
)"""";

constexpr std::string_view SCRIPT_WITH_NO_SCRIPT = R""""(
method: "script"
sun_poller: "sunwait"
background_config: "./image"
)"""";

std::expected<Config, ConfigError>
loadConfigFromString(const std::string_view configString) {
  return loadConfigFromYAML(YAML::Load(std::string(configString)));
}

} // namespace

TEST(GeneralConfig, AllFilled) {
  const std::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(GENERAL_CONFIG_YAML);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &cfg = expectedConfig.value();

  EXPECT_EQ(
      std::get<BackgroundSetterMethodWallUtils>(cfg.backgroundSetterMethod),
      BackgroundSetterMethodWallUtils());
  EXPECT_EQ(cfg.sunEventPollerMethod, SunEventPollerMethod::Sunwait);
  EXPECT_EQ(cfg.backgroundSetConfigFile,
            std::filesystem::path("./an_image_dir"));
  EXPECT_EQ(cfg.hookScript,
            std::make_optional(std::filesystem::path("./hook_script.sh")));
  EXPECT_EQ(cfg.imageCacheDirectory,
            std::filesystem::path("~/.cache/backgrounds"));
}

TEST(GeneralConfig, DefaultValues) {
  const std::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(EMPTY_YAML);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &cfg = expectedConfig.value();

  EXPECT_EQ(
      std::get<BackgroundSetterMethodWallUtils>(cfg.backgroundSetterMethod),
      BackgroundSetterMethodWallUtils());
  EXPECT_EQ(cfg.sunEventPollerMethod, SunEventPollerMethod::Sunwait);
  EXPECT_EQ(cfg.backgroundSetConfigFile,
            std::filesystem::path(DEFAULT_BACKGROUND_SET_CONFIG));
  EXPECT_EQ(cfg.hookScript, std::nullopt);
  EXPECT_EQ(cfg.imageCacheDirectory,
            std::filesystem::path(DEFAULT_CACHE_IMAGE_DIR));
}

TEST(GeneralConfig, MethodOnly) {
  const std::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(GENERAL_CONFIG_ONLY_METHOD_YAML);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &cfg = expectedConfig.value();

  EXPECT_EQ(
      std::get<BackgroundSetterMethodWallUtils>(cfg.backgroundSetterMethod),
      BackgroundSetterMethodWallUtils());
  EXPECT_EQ(cfg.sunEventPollerMethod, SunEventPollerMethod::Sunwait);
  EXPECT_EQ(cfg.backgroundSetConfigFile,
            std::filesystem::path(DEFAULT_BACKGROUND_SET_CONFIG));
  EXPECT_EQ(cfg.hookScript, std::nullopt);
}

TEST(GeneralConfig, MethodWallUtils) {
  const std::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(METHOD_WALLUTILS);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &cfg = expectedConfig.value();

  EXPECT_EQ(
      std::get<BackgroundSetterMethodWallUtils>(cfg.backgroundSetterMethod),
      BackgroundSetterMethodWallUtils());

  EXPECT_EQ(cfg.sunEventPollerMethod, SunEventPollerMethod::Sunwait);
  EXPECT_EQ(cfg.backgroundSetConfigFile, std::filesystem::path("./image"));
  EXPECT_EQ(cfg.hookScript, std::nullopt);
}

TEST(GeneralConfig, MethodScript) {
  const std::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(METHOD_SCRIPT);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &cfg = expectedConfig.value();

  EXPECT_EQ(std::get<BackgroundSetterMethodScript>(cfg.backgroundSetterMethod),
            BackgroundSetterMethodScript(std::filesystem::path("./script.sh")));

  EXPECT_EQ(cfg.sunEventPollerMethod, SunEventPollerMethod::Sunwait);
  EXPECT_EQ(cfg.backgroundSetConfigFile, std::filesystem::path("./image"));
  EXPECT_EQ(cfg.hookScript, std::nullopt);
}

TEST(GeneralConfig, BothMethodAndScript) {
  const std::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(BOTH_METHOD_AND_SCRIPT);

  EXPECT_TRUE(expectedConfig.has_value());

  const Config &cfg = expectedConfig.value();

  EXPECT_EQ(
      std::get<BackgroundSetterMethodWallUtils>(cfg.backgroundSetterMethod),
      BackgroundSetterMethodWallUtils());

  EXPECT_EQ(cfg.sunEventPollerMethod, SunEventPollerMethod::Sunwait);
  EXPECT_EQ(cfg.backgroundSetConfigFile, std::filesystem::path("./image"));
  EXPECT_EQ(cfg.hookScript, std::nullopt);
}

TEST(GeneralConfig, ScriptWithNoScript) {
  const std::expected<Config, ConfigError> expectedConfig =
      loadConfigFromString(SCRIPT_WITH_NO_SCRIPT);

  EXPECT_FALSE(expectedConfig.has_value());

  EXPECT_EQ(expectedConfig.error(), ConfigError::MethodParsingError);
}
