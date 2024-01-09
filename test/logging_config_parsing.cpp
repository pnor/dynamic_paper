/**
 *   Test parsing of YAML config files to set the logging level
 */

#include <gtest/gtest.h>

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include "src/config.hpp"

using namespace dynamic_paper;

class GeneralConfigLogging : public testing::Test {
  std::optional<spdlog::level::level_enum> globalLogLevel;

protected:
  void SetUp() override { globalLogLevel = spdlog::get_level(); }

  void TearDown() override {
    if (globalLogLevel.has_value()) {
      spdlog::set_level(globalLogLevel.value());
    }
  }
};

static const std::string CONFIG_NO_LOGGING = R""""(
method: "wallutils"
sun_poller: "sunwait"
image_dir: "./an_image_dir"
)"""";

static const std::string CONFIG_INFO_LOGGING = R""""(
method: "wallutils"
sun_poller: "sunwait"
image_dir: "./an_image_dir"
logging_level: "info"
)"""";

static const std::string CONFIG_WARNING_LOGGING = R""""(
method: "wallutils"
sun_poller: "sunwait"
image_dir: "./an_image_dir"
logging_level: "warning"
)"""";

static const std::string CONFIG_ERROR_LOGGING = R""""(
method: "wallutils"
sun_poller: "sunwait"
image_dir: "./an_image_dir"
logging_level: "error"
)"""";

static const std::string CONFIG_DEBUG_LOGGING = R""""(
method: "wallutils"
sun_poller: "sunwait"
image_dir: "./an_image_dir"
logging_level: "debug"
)"""";

static const std::string CONFIG_CRITICAL_LOGGING = R""""(
method: "wallutils"
sun_poller: "sunwait"
image_dir: "./an_image_dir"
logging_level: "critical"
)"""";

static const std::string CONFIG_TRACE_LOGGING = R""""(
method: "wallutils"
sun_poller: "sunwait"
image_dir: "./an_image_dir"
logging_level: "trace"
)"""";

static const std::string CONFIG_OFF_LOGGING = R""""(
method: "wallutils"
sun_poller: "sunwait"
image_dir: "./an_image_dir"
logging_level: "off"
)"""";

TEST(GeneralConfigLogging, ConfigNoLogging) {
  LogLevel level = loadLoggingLevelFromYAML(YAML::Load(CONFIG_NO_LOGGING));
  EXPECT_EQ(level, ConfigDefaults::logLevel);
}

TEST(GeneralConfigLogging, ConfigLoggingInfo) {
  LogLevel level = loadLoggingLevelFromYAML(YAML::Load(CONFIG_INFO_LOGGING));
  EXPECT_EQ(level, LogLevel::INFO);
}

TEST(GeneralConfigLogging, ConfigLoggingWarning) {
  LogLevel level = loadLoggingLevelFromYAML(YAML::Load(CONFIG_WARNING_LOGGING));
  EXPECT_EQ(level, LogLevel::WARNING);
}

TEST(GeneralConfigLogging, ConfigLoggingError) {
  LogLevel level = loadLoggingLevelFromYAML(YAML::Load(CONFIG_ERROR_LOGGING));
  EXPECT_EQ(level, LogLevel::ERROR);
}

TEST(GeneralConfigLogging, ConfigLoggingDebug) {
  LogLevel level = loadLoggingLevelFromYAML(YAML::Load(CONFIG_DEBUG_LOGGING));
  EXPECT_EQ(level, LogLevel::DEBUG);
}

TEST(GeneralConfigLogging, ConfigLoggingCritical) {
  LogLevel level =
      loadLoggingLevelFromYAML(YAML::Load(CONFIG_CRITICAL_LOGGING));
  EXPECT_EQ(level, LogLevel::CRITICAL);
}

TEST(GeneralConfigLogging, ConfigLoggingTrace) {
  LogLevel level = loadLoggingLevelFromYAML(YAML::Load(CONFIG_TRACE_LOGGING));
  EXPECT_EQ(level, LogLevel::TRACE);
}

TEST(GeneralConfigLogging, ConfigLoggingOff) {
  LogLevel level = loadLoggingLevelFromYAML(YAML::Load(CONFIG_OFF_LOGGING));
  EXPECT_EQ(level, LogLevel::OFF);
}
