/**
 *   Test parsing of YAML config files to set the logging level
 */

#include <gtest/gtest.h>

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include "src/config.hpp"
#include "src/defaults.hpp"

using namespace dynamic_paper;

namespace {

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

const std::string_view CONFIG_NO_LOGGING = R""""(
method: "wallutils"
sun_poller: "sunwait"
)"""";

const std::string_view CONFIG_INFO_LOGGING = R""""(
method: "wallutils"
sun_poller: "sunwait"
logging_level: "info"
)"""";

const std::string_view CONFIG_WARNING_LOGGING = R""""(
method: "wallutils"
sun_poller: "sunwait"
logging_level: "warning"
)"""";

const std::string_view CONFIG_ERROR_LOGGING = R""""(
method: "wallutils"
sun_poller: "sunwait"
logging_level: "error"
)"""";

const std::string_view CONFIG_DEBUG_LOGGING = R""""(
method: "wallutils"
sun_poller: "sunwait"
logging_level: "debug"
)"""";

const std::string_view CONFIG_CRITICAL_LOGGING = R""""(
method: "wallutils"
sun_poller: "sunwait"
logging_level: "critical"
)"""";

const std::string_view CONFIG_TRACE_LOGGING = R""""(
method: "wallutils"
sun_poller: "sunwait"
logging_level: "trace"
)"""";

const std::string_view CONFIG_OFF_LOGGING = R""""(
method: "wallutils"
sun_poller: "sunwait"
logging_level: "off"
)"""";

const std::string_view CONFIG_WITH_CONFIG_FILE = R""""(
method: "wallutils"
sun_poller: "sunwait"
logging_level: "off"
log_file: "config_location"
)"""";

const std::string_view CONFIG_NO_CONFIG_FILE = R""""(
method: "wallutils"
sun_poller: "sunwait"
logging_level: "off"
)"""";

LogLevel getLogLevelFromYAML(const YAML::Node &yaml) {
  std::pair<LogLevel, std::filesystem::path> levelAndFile =
      loadLoggingInfoFromYAML(yaml);
  return levelAndFile.first;
}

std::string getConfigFileNameFromYAML(const YAML::Node &yaml) {
  std::pair<LogLevel, std::filesystem::path> levelAndFile =
      loadLoggingInfoFromYAML(yaml);
  return levelAndFile.second;
}

} // namespace

TEST(GeneralConfigLogging, ConfigNoLogging) {
  // TODO fix this
  const LogLevel level =
      getLogLevelFromYAML(YAML::Load(std::string(CONFIG_NO_LOGGING)));
  EXPECT_EQ(level, ConfigDefaults::logLevel);
}

TEST(GeneralConfigLogging, ConfigLoggingInfo) {
  const LogLevel level =
      getLogLevelFromYAML(YAML::Load(std::string(CONFIG_INFO_LOGGING)));
  EXPECT_EQ(level, LogLevel::INFO);
}

TEST(GeneralConfigLogging, ConfigLoggingWarning) {
  const LogLevel level =
      getLogLevelFromYAML(YAML::Load(std::string(CONFIG_WARNING_LOGGING)));
  EXPECT_EQ(level, LogLevel::WARNING);
}

TEST(GeneralConfigLogging, ConfigLoggingError) {
  const LogLevel level =
      getLogLevelFromYAML(YAML::Load(std::string(CONFIG_ERROR_LOGGING)));
  EXPECT_EQ(level, LogLevel::ERROR);
}

TEST(GeneralConfigLogging, ConfigLoggingDebug) {
  const LogLevel level =
      getLogLevelFromYAML(YAML::Load(std::string(CONFIG_DEBUG_LOGGING)));
  EXPECT_EQ(level, LogLevel::DEBUG);
}

TEST(GeneralConfigLogging, ConfigLoggingCritical) {
  const LogLevel level =
      getLogLevelFromYAML(YAML::Load(std::string(CONFIG_CRITICAL_LOGGING)));
  EXPECT_EQ(level, LogLevel::CRITICAL);
}

TEST(GeneralConfigLogging, ConfigLoggingTrace) {
  const LogLevel level =
      getLogLevelFromYAML(YAML::Load(std::string(CONFIG_TRACE_LOGGING)));
  EXPECT_EQ(level, LogLevel::TRACE);
}

TEST(GeneralConfigLogging, ConfigLoggingOff) {
  const LogLevel level =
      getLogLevelFromYAML(YAML::Load(std::string(CONFIG_OFF_LOGGING)));
  EXPECT_EQ(level, LogLevel::OFF);
}

TEST(GeneralConfigLogging, HasConfigFile) {
  const std::filesystem::path logFile = getConfigFileNameFromYAML(
      YAML::Load(std::string(CONFIG_WITH_CONFIG_FILE)));
  EXPECT_EQ(logFile, std::filesystem::path("config_location"));
}

TEST(GeneralConfigLogging, HasNoConfigFile) {
  const std::filesystem::path logFile =
      getConfigFileNameFromYAML(YAML::Load(std::string(CONFIG_NO_CONFIG_FILE)));
  EXPECT_EQ(logFile, ConfigDefaults::logFileName());
}
