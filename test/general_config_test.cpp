/**
 *   Test parsing of YAML config files
 */

#include <gtest/gtest.h>

#include <yaml-cpp/yaml.h>

#include "src/config.hpp"

using namespace dynamic_paper;

static const std::string GENERAL_CONFIG_YAML = R""""(
method: "wallutils"
sun_poller: "sunwait"
image_dir: "./an_image_dir"
hook_script: "./hook_script.sh"
)"""";

static const std::string EMPTY_YAML = "";

static const std::string GENERAL_CONFIG_ONLY_METHOD_YAML = R""""(
method: "script"
)"""";

TEST(GeneralConfig, AllFilled) {
  Config cfg = loadConfigFromYAML(YAML::Load(GENERAL_CONFIG_YAML));

  EXPECT_EQ(cfg.backgroundSetterMethod, BackgroundSetterMethod::WallUtils);
  EXPECT_EQ(cfg.sunEventPollerMethod, SunEventPollerMethod::Sunwait);
  EXPECT_EQ(cfg.backgroundImageDir, std::filesystem::path("./an_image_dir"));
  EXPECT_EQ(cfg.hookScript,
            std::make_optional(std::filesystem::path("./hook_script.sh")));
}

TEST(GeneralConfig, DefaultValues) {
  Config cfg = loadConfigFromYAML(YAML::Load(EMPTY_YAML));

  EXPECT_EQ(cfg.backgroundSetterMethod, BackgroundSetterMethod::WallUtils);
  EXPECT_EQ(cfg.sunEventPollerMethod, SunEventPollerMethod::Sunwait);
  EXPECT_EQ(cfg.backgroundImageDir,
            std::filesystem::path("~/.local/share/dynamic_paper/images"));
  EXPECT_EQ(cfg.hookScript, std::nullopt);
}

TEST(GeneralConfig, MethodOnly) {
  Config cfg = loadConfigFromYAML(YAML::Load(GENERAL_CONFIG_ONLY_METHOD_YAML));

  EXPECT_EQ(cfg.backgroundSetterMethod, BackgroundSetterMethod::Script);
  EXPECT_EQ(cfg.sunEventPollerMethod, SunEventPollerMethod::Sunwait);
  EXPECT_EQ(cfg.backgroundImageDir,
            std::filesystem::path("~/.local/share/dynamic_paper/images"));
  EXPECT_EQ(cfg.hookScript, std::nullopt);
}
