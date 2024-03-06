#pragma once

#include <expected>
#include <filesystem>
#include <optional>
#include <variant>

#include <tl/expected.hpp>
#include <yaml-cpp/yaml.h>

#include "background_set_enums.hpp"
#include "config.hpp"
#include "dynamic_background_set.hpp"
#include "static_background_set.hpp"

/** Object that represents a static/dynamic background to be shown */
namespace dynamic_paper {

/** Collection of information used to determine how to display a background.
 * Identified by its `name` */
class BackgroundSet {
public:
  [[nodiscard]] std::string_view getName() const;
  [[nodiscard]] BackgroundSetType getType() const;

  [[nodiscard]] std::optional<StaticBackgroundData> getStaticBackgroundData();

  [[nodiscard]] std::optional<DynamicBackgroundData> getDynamicBackgroundData();

  BackgroundSet(std::string name, StaticBackgroundData data);
  BackgroundSet(std::string name, DynamicBackgroundData data);

private:
  std::string name;
  std::variant<StaticBackgroundData, DynamicBackgroundData> type;
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

tl::expected<BackgroundSet, BackgroundSetParseErrors>
parseFromYAML(const std::string &name, const YAML::Node &yaml,
              const Config &config);
} // namespace dynamic_paper
