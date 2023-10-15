#pragma once

#include <filesystem>
#include <optional>
#include <variant>

#include <yaml-cpp/yaml.h>

#include "background_set_enums.hpp"

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
  std::vector<unsigned int> times;

  DynamicBackgroundData(std::filesystem::path dataDirectory,
                        BackgroundSetMode mode,
                        std::optional<unsigned int> transitionDuration,
                        BackgroundSetOrder order,
                        std::vector<std::string> imageNames,
                        std::vector<unsigned int> times);
};

class BackgroundSet {
public:
  BackgroundSet(std::string name, StaticBackgroundData data);
  BackgroundSet(std::string name, DynamicBackgroundData data);

private:
  std::string name;
  std::variant<StaticBackgroundData, DynamicBackgroundData> type;
};

/** Can raise an exception if unable to parse valid BackgroundSet */
BackgroundSet parseFromYAML(YAML::Node yaml);
} // namespace dynamic_paper
