#pragma once

#include <expected>
#include <filesystem>
#include <optional>
#include <variant>

#include <yaml-cpp/yaml.h>

#include "background_set_enums.hpp"
#include "config.hpp"

/** Object that represents a static/dynamic background to be shown */
namespace dynamic_paper {

struct StaticBackgroundData {
  std::filesystem::path dataDirectory;
  BackgroundSetMode mode;
  std::vector<std::string> imageNames;

  StaticBackgroundData(std::filesystem::path dataDirectory,
                       BackgroundSetMode mode,
                       std::vector<std::string> imageNames);
};

struct DynamicBackgroundData {
  std::filesystem::path dataDirectory;
  BackgroundSetMode mode;
  /** nullopt if does not transition. # seconds otherwise. */
  std::optional<unsigned int> transitionDuration;
  BackgroundSetOrder order;
  std::vector<std::string> imageNames;
  /** each entry represents number seconds after 00:00 to do a transition */
  std::vector<time_t> times;

  DynamicBackgroundData(std::filesystem::path dataDirectory,
                        BackgroundSetMode mode,
                        std::optional<unsigned int> transitionDuration,
                        BackgroundSetOrder order,
                        std::vector<std::string> imageNames,
                        std::vector<time_t> times);
};

class BackgroundSet {
public:
  std::string name;
  std::variant<StaticBackgroundData, DynamicBackgroundData> type;

  BackgroundSet(std::string name, StaticBackgroundData data);
  BackgroundSet(std::string name, DynamicBackgroundData data);
};

enum class BackgroundSetParseErrors { MissingSunpollInfo, BadTimes };

std::expected<BackgroundSet, BackgroundSetParseErrors>
parseFromYAML(const std::string &name, const YAML::Node &yaml,
              const Config &config);
} // namespace dynamic_paper
