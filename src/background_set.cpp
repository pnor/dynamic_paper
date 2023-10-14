#include "background_set.hpp"

#include <type_traits>

#include "yaml_helper.hpp"

namespace dynamic_paper {

static constexpr std::string_view DATA_DIRECTORY = "data_directory";
static constexpr std::string_view IMAGES = "images";
static constexpr std::string_view IMAGE = "image";
static constexpr std::string_view MODE = "mode";
static constexpr std::string_view ORDER = "order";
static constexpr std::string_view TIMES = "times";
static constexpr std::string_view TRANSITION = "transition";
static constexpr std::string_view TRANSITION_LENGTH = "transition_length";
static constexpr std::string_view TYPE = "type";

// ===== Helper ===============

template <typename T>
static void insertIntoParsingInfo(const YAML::const_iterator yaml,
                                  std::optional<T> &field) {
  if constexpr (std::is_same_v<T, std::string>) {
    T yamlData = yaml->second.as<T>();
    field = std::make_optional(yamlData);
  } else {
    std::optional<T> optData = stringTo<T>(yaml->second.as<std::string>());
    if (optData.has_value()) {
      field = optData;
    }
  }
}

// Version of Helper function to put lists into ParsingInfo
template <typename T>
static void insertIntoParsingInfo(const YAML::const_iterator yaml,
                                  std::optional<std::vector<T>> &field) {
  std::vector<T> yamlItems;

  for (YAML::const_iterator it = yaml->second.begin(); it != yaml->second.end();
       ++it) {
    if (!it->IsScalar()) {
      continue;
    }
    yamlItems.push_back(it->as<T>());
  }

  field = std::make_optional<std::vector<T>>(std::move(yamlItems));
}

// ===== Constructor ===============

StaticBackgroundData::StaticBackgroundData(std::filesystem::path dataDirectory,
                                           BackgroundSetMode mode,
                                           std::vector<std::string> imageNames)
    : dataDirectory(dataDirectory), mode(mode), imageNames(imageNames) {}

DynamicBackgroundData::DynamicBackgroundData(
    std::filesystem::path dataDirectory, BackgroundSetMode mode,
    std::optional<unsigned int> transitionDuration, BackgroundSetOrder order,
    std::vector<std::string> imageNames, std::vector<unsigned int> times)
    : dataDirectory(dataDirectory), mode(mode),
      transitionDuration(transitionDuration), order(order),
      imageNames(imageNames), times(times) {}

BackgroundSet::BackgroundSet(std::string name, StaticBackgroundData data)
    : name(name),
      type(std::variant<StaticBackgroundData, DynamicBackgroundData>(data)) {}

BackgroundSet::BackgroundSet(std::string name, DynamicBackgroundData data)
    : name(name),
      type(std::variant<StaticBackgroundData, DynamicBackgroundData>(data)) {}

// ===== Parsing ==================
/** Information parsed from the body of the yaml as it reads */
struct ParsingInfo {
  std::optional<std::string> name = std::nullopt;
  std::optional<std::filesystem::path> dataDirectory = std::nullopt;
  std::optional<BackgroundSetType> type = std::nullopt;
  std::optional<BackgroundSetMode> mode = std::nullopt;
  std::optional<bool> transition = std::nullopt;
  std::optional<unsigned int> transitionLength = std::nullopt;
  std::optional<BackgroundSetOrder> order = std::nullopt;
  std::optional<std::vector<std::string>> images = std::nullopt;
  std::optional<std::string> image = std::nullopt;
  std::optional<std::string> timeStrings = std::nullopt;
};

static void updateParsingInfoWithYamlNode(const YAML::const_iterator yaml,
                                          ParsingInfo &parsingInfo) {
  if (!yaml->IsMap()) {
    return;
  }

  const std::string &key = yaml->first.as<std::string>();

  if (key == DATA_DIRECTORY && yaml->second.IsScalar()) {
    insertIntoParsingInfo<std::filesystem::path>(yaml,
                                                 parsingInfo.dataDirectory);
  } else if (key == IMAGES && yaml->second.IsSequence()) {
    insertIntoParsingInfo<std::vector<std::string>>(yaml, parsingInfo.images);
  } else if (key == IMAGE) {
    insertIntoParsingInfo<std::string>(yaml, parsingInfo.image);
  } else if (key == MODE) {
    insertIntoParsingInfo<BackgroundSetMode>(yaml, parsingInfo.mode);
  } else if (key == ORDER) {
    insertIntoParsingInfo<BackgroundSetOrder>(yaml, parsingInfo.order);
  } else if (key == TIMES) {
    insertIntoParsingInfo<std::string>(yaml, parsingInfo.timeStrings);
  } else if (key == TRANSITION) {
    insertIntoParsingInfo<bool>(yaml, parsingInfo.transition);
  } else if (key == TRANSITION_LENGTH) {
    insertIntoParsingInfo<unsigned int>(yaml, parsingInfo.transitionLength);
  } else if (key == TYPE) {
    insertIntoParsingInfo<BackgroundSetType>(yaml, parsingInfo.type);
  }
}

std::optional<BackgroundSet>
createBackgroundSetFromInfo(const ParsingInfo &parsingInfo) {
  if (!parsingInfo.type.has_value()) {
    return std::nullopt;
  }
}

std::optional<BackgroundSet> parseFromYAML(YAML::const_iterator yaml) {
  ParsingInfo parsingInfo;

  parsingInfo.name = std::make_optional(yaml->first.as<std::string>());

  for (YAML::const_iterator it = yaml->second.begin(); it != yaml->second.end();
       ++it) {
    updateParsingInfoWithYamlNode(it, parsingInfo);
  }
}
} // namespace dynamic_paper
