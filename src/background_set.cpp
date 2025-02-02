#include "background_set.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

#include <tl/expected.hpp>
#include <yaml-cpp/node/node.h>

#include "background_set_enums.hpp"
#include "constants.hpp"
#include "defaults.hpp"
#include "dynamic_background_set.hpp"
#include "file_util.hpp"
#include "logger.hpp"
#include "static_background_set.hpp"
#include "transition_info.hpp"
#include "type_helper.hpp"
#include "yaml_helper.hpp"

// TODO make this easier to add and remove keys; should be able to just list it
// in a struct that specifies the key parsed, into a type

namespace dynamic_paper {

namespace {

// ===== Helper ===============

/** Parses a field from `yaml` and puts it into `field`. */
template <typename T>
void insertIntoParsingInfo(const YAML::Node &yaml, std::optional<T> &field);

// String
template <typename T>
  requires(std::is_same_v<T, std::string>)
void insertIntoParsingInfo(const YAML::Node &yaml, std::optional<T> &field) {
  auto yamlData = yaml.as<std::string>();
  field = std::make_optional(yamlData);
}

// Vector
template <typename T>
  requires(is_vector<T>::value)
void insertIntoParsingInfo(const YAML::Node &yaml, std::optional<T> &field) {
  T yamlData;
  yamlData.reserve(yaml.size());

  for (const auto &yamlField : yaml) {
    yamlData.push_back(yamlField.as<typename T::value_type>());
  }

  field = std::make_optional(yamlData);
}

// Path
template <typename T>
  requires(std::is_same_v<T, std::filesystem::path>)
void insertIntoParsingInfo(const YAML::Node &yaml, std::optional<T> &field) {
  using path = std::filesystem::path;

  const std::optional<path> optData =
      yamlStringTo<path>(yaml.as<std::string>());

  if (optData.has_value()) {
    const path expandedPath = expandPath(optData.value());
    field = expandedPath;
  }
}

// Default
template <typename T>
void insertIntoParsingInfo(const YAML::Node &yaml, std::optional<T> &field) {
  std::optional<T> optData = yamlStringTo<T>(yaml.as<std::string>());
  if (optData.has_value()) {
    field = std::move(optData);
  }
}

// ===== Parsing Helper ==================
/** Information parsed from the body of the yaml as it reads */
struct ParsingInfo {
  std::optional<std::string> name = std::nullopt;
  std::optional<std::filesystem::path> imageDirectory = std::nullopt;
  std::optional<BackgroundSetType> type = std::nullopt;
  std::optional<BackgroundSetMode> mode = std::nullopt;
  std::optional<unsigned int> transitionLength = std::nullopt;
  std::optional<BackgroundSetOrder> order = std::nullopt;
  std::optional<std::vector<std::string>> images = std::nullopt;
  std::optional<std::string> image = std::nullopt;
  std::optional<std::vector<std::string>> timeStrings = std::nullopt;
  std::optional<unsigned int> numberTransitionSteps = std::nullopt;
  std::optional<bool> inPlace = std::nullopt;
};

void updateParsingInfoWithYamlNode(const std::string &key,
                                   const YAML::Node &value,
                                   ParsingInfo &parsingInfo) {
  if (key == IMAGE_DIRECTORY && value.IsScalar()) {
    insertIntoParsingInfo<std::filesystem::path>(value,
                                                 parsingInfo.imageDirectory);
  } else if (key == IMAGES && value.IsSequence()) {
    insertIntoParsingInfo<std::vector<std::string>>(value, parsingInfo.images);
  } else if (key == IMAGE) {
    insertIntoParsingInfo<std::string>(value, parsingInfo.image);
  } else if (key == MODE) {
    insertIntoParsingInfo<BackgroundSetMode>(value, parsingInfo.mode);
  } else if (key == ORDER) {
    insertIntoParsingInfo<BackgroundSetOrder>(value, parsingInfo.order);
  } else if (key == TIMES) {
    insertIntoParsingInfo<std::vector<std::string>>(value,
                                                    parsingInfo.timeStrings);
  } else if (key == TRANSITION_LENGTH) {
    insertIntoParsingInfo<unsigned int>(value, parsingInfo.transitionLength);
  } else if (key == TYPE) {
    insertIntoParsingInfo<BackgroundSetType>(value, parsingInfo.type);
  } else if (key == NUM_TRANSITION_STEPS) {
    insertIntoParsingInfo<unsigned int>(value,
                                        parsingInfo.numberTransitionSteps);
  } else if (key == IN_PLACE) {
    insertIntoParsingInfo<bool>(value, parsingInfo.inPlace);
  }
}

std::optional<TransitionInfo>
tryCreateTransitionInfoFrom(const ParsingInfo &parsingInfo) {
  if (parsingInfo.transitionLength.has_value() &&
      parsingInfo.numberTransitionSteps.has_value()) {

    if (parsingInfo.transitionLength.value() <= 0) {
      logWarning("Transition Length was <= 0, so not creating transitions");
      return std::nullopt;
    }

    return std::make_optional<TransitionInfo>(
        std::chrono::seconds(parsingInfo.transitionLength.value()),
        parsingInfo.numberTransitionSteps.value(),
        parsingInfo.inPlace.value_or(false));
  }

  if (!parsingInfo.numberTransitionSteps.has_value() &&
      parsingInfo.transitionLength.has_value()) {
    logWarning(
        "No number of transition steps was provided so using default steps");
    return std::make_optional<TransitionInfo>(
        std::chrono::seconds(parsingInfo.transitionLength.value()),
        BackgroundSetDefaults::transitionSteps,
        parsingInfo.inPlace.value_or(false));
  }
  if (parsingInfo.numberTransitionSteps.has_value() &&
      !parsingInfo.transitionLength.has_value()) {
    logError("Cannot make transition with only number transition steps and no "
             "transition length");
  }

  return std::nullopt;
}

tl::expected<BackgroundSet, BackgroundSetParseErrors>
createStaticBackgroundSetFromInfo(const ParsingInfo &parsingInfo) {
  assert((void("Parsing info should have name by time this helper function "
               "is called"),
          parsingInfo.name.has_value()));

  const std::string &name = parsingInfo.name.value();

  if (!(parsingInfo.image.has_value() || parsingInfo.images.has_value())) {
    if (parsingInfo.images.has_value() && parsingInfo.images->empty()) {
      return tl::unexpected(BackgroundSetParseErrors::NoImages);
    }
    if (!parsingInfo.image.has_value()) {
      return tl::unexpected(BackgroundSetParseErrors::NoImages);
    }
  }

  if (!parsingInfo.imageDirectory.has_value()) {
    return tl::unexpected(BackgroundSetParseErrors::NoImageDirectory);
  }

  logAssert(parsingInfo.image.has_value() || parsingInfo.images.has_value(),
            "Parsing info must have an image");

  if (parsingInfo.images.has_value()) {
    return BackgroundSet(
        name, StaticBackgroundData(
                  parsingInfo.imageDirectory.value(),
                  parsingInfo.mode.value_or(BackgroundSetDefaults::mode),
                  parsingInfo.images.value()));
  }

  if (parsingInfo.image.has_value()) {
    return BackgroundSet(
        name, StaticBackgroundData(
                  parsingInfo.imageDirectory.value(),
                  parsingInfo.mode.value_or(BackgroundSetDefaults::mode),
                  {parsingInfo.image.value()}));
  }

  throw std::logic_error("Got to end of Static BackgroundSet with both "
                         "image and images unset");
}

tl::expected<BackgroundSet, BackgroundSetParseErrors>
createDynamicBackgroundSetFromInfo(const ParsingInfo &parsingInfo,
                                   const SolarDay &solarDay) {
  assert((void("Parsing info should have name by time this helper function "
               "is called"),
          parsingInfo.name.has_value()));

  const std::string &name = parsingInfo.name.value();

  if (!parsingInfo.images.has_value() || parsingInfo.images->empty()) {
    return tl::unexpected(BackgroundSetParseErrors::NoImages);
  }

  if (!parsingInfo.imageDirectory.has_value()) {
    return tl::unexpected(BackgroundSetParseErrors::NoImageDirectory);
  }

  if (!parsingInfo.timeStrings.has_value() ||
      parsingInfo.timeStrings->empty()) {
    return tl::unexpected(BackgroundSetParseErrors::NoTimes);
  }

  logDebug("Sunrise time is {} and Sunset time is {}", solarDay.sunrise,
           solarDay.sunset);

  std::optional<std::vector<TimeFromMidnight>> optTimeOffsets =
      timeStringsToTimes(parsingInfo.timeStrings.value(), solarDay);
  if (!optTimeOffsets.has_value()) {
    return tl::unexpected(BackgroundSetParseErrors::BadTimes);
  }

  const std::optional<TransitionInfo> transition =
      tryCreateTransitionInfoFrom(parsingInfo);

  return BackgroundSet(
      name,
      DynamicBackgroundData(
          parsingInfo.imageDirectory.value(),
          parsingInfo.mode.value_or(BackgroundSetDefaults::mode), transition,
          parsingInfo.order.value_or(BackgroundSetDefaults::order),
          parsingInfo.images.value(), optTimeOffsets.value()));
}

tl::expected<BackgroundSet, BackgroundSetParseErrors>
createBackgroundSetFromInfo(const ParsingInfo &parsingInfo,
                            const SolarDay &solarDay) {
  if (!parsingInfo.name.has_value()) {
    return tl::unexpected(BackgroundSetParseErrors::NoName);
  }

  if (!parsingInfo.type.has_value()) {
    return tl::unexpected(BackgroundSetParseErrors::NoType);
  }

  switch (parsingInfo.type.value()) {
  case BackgroundSetType::Static: {
    return createStaticBackgroundSetFromInfo(parsingInfo);
  }
  case BackgroundSetType::Dynamic: {
    return createDynamicBackgroundSetFromInfo(parsingInfo, solarDay);
  }
  }

  throw std::logic_error("Unhandled background set type");
}

} // namespace

// ===== Header ===============

BackgroundSet::BackgroundSet(std::string name, StaticBackgroundData data)
    : name(std::move(name)),
      type(std::variant<StaticBackgroundData, DynamicBackgroundData>(data)) {}

BackgroundSet::BackgroundSet(std::string name, DynamicBackgroundData data)
    : name(std::move(name)),
      type(std::variant<StaticBackgroundData, DynamicBackgroundData>(data)) {}

std::string_view BackgroundSet::getName() const { return this->name; }

BackgroundSetType BackgroundSet::getType() const {
  return std::visit(overloaded{[](const StaticBackgroundData & /*event*/) {
                                 return BackgroundSetType::Static;
                               },
                               [](const DynamicBackgroundData & /*event*/) {
                                 return BackgroundSetType::Dynamic;
                               }},
                    this->type);
}

std::optional<StaticBackgroundData>
BackgroundSet::getStaticBackgroundData() const {
  const auto *staticDataPtr = std::get_if<StaticBackgroundData>(&this->type);

  if (staticDataPtr == nullptr) {
    return std::nullopt;
  }
  return *staticDataPtr;
}

std::optional<DynamicBackgroundData>
BackgroundSet::getDynamicBackgroundData() const {
  const auto *dynamicDataPtr = std::get_if<DynamicBackgroundData>(&this->type);

  if (dynamicDataPtr == nullptr) {
    return std::nullopt;
  }
  return *dynamicDataPtr;
}

tl::expected<BackgroundSet, BackgroundSetParseErrors>
parseFromYAML(const std::string &name, const YAML::Node &yaml,
              const SolarDay &solarDay) {
  ParsingInfo parsingInfo;

  const auto yamlMap = yaml.as<std::unordered_map<std::string, YAML::Node>>();

  parsingInfo.name = std::make_optional(name);

  for (const auto &keyYamlVal : yamlMap) {
    updateParsingInfoWithYamlNode(keyYamlVal.first, keyYamlVal.second,
                                  parsingInfo);
  }

  return createBackgroundSetFromInfo(parsingInfo, solarDay);
}
} // namespace dynamic_paper
