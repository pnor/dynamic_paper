#pragma once

#include <expected>
#include <filesystem>
#include <optional>
#include <variant>

#include <yaml-cpp/yaml.h>

#include "background_set_enums.hpp"
#include "config.hpp"
#include "dynamic_background_set.hpp"
#include "static_background_set.hpp"

/** Object that represents a static/dynamic background to be shown */
namespace dynamic_paper {

class BackgroundSet {
public:
  std::string name;
  std::variant<StaticBackgroundData, DynamicBackgroundData> type;

  BackgroundSet(std::string name, StaticBackgroundData data);
  BackgroundSet(std::string name, DynamicBackgroundData data);

  void show(const BackgroundSetterMethod &method, const Config &config) const;
};

enum class BackgroundSetParseErrors {
  NoName,
  NoType,
  MissingSunpollInfo,
  NoImageDirectory,
  BadTimes,
  NoImages,
  NoTimes
};

std::expected<BackgroundSet, BackgroundSetParseErrors>
parseFromYAML(const std::string &name, const YAML::Node &yaml,
              const Config &config);
} // namespace dynamic_paper
