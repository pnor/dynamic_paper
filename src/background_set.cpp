#include "background_set.hpp"

#include <algorithm>
#include <cassert>
#include <format>
#include <type_traits>

#include "yaml_helper.hpp"

namespace dynamic_paper {

// YAML Dict Key
static constexpr std::string_view DATA_DIRECTORY = "data_directory";
static constexpr std::string_view IMAGES = "images";
static constexpr std::string_view IMAGE = "image";
static constexpr std::string_view MODE = "mode";
static constexpr std::string_view ORDER = "order";
static constexpr std::string_view TIMES = "times";
static constexpr std::string_view TRANSITION = "transition";
static constexpr std::string_view TRANSITION_LENGTH = "transition_length";
static constexpr std::string_view TYPE = "type";
// Default value
static constexpr BackgroundSetMode DEFAULT_MODE = BackgroundSetMode::Center;
static constexpr BackgroundSetOrder DEFAULT_ORDER = BackgroundSetOrder::Linear;

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

template <typename T>
static void checkParsingInfoHas(const T &field, const std::string &errorMsg) {
  if (!field.has_value()) {
    throw std::runtime_error(errorMsg);
  }
}

template <typename T, typename U>
static void checkParsingInfoHasEither(const T &field1, const U &field2,
                                      const std::string &errorMsg) {
  if (field1.has_value() && field2.has_value()) {
    throw std::runtime_error(errorMsg);
  }
}

unsigned int timeStringToTimeOffset(const std::string &s) {
  return 0;
  // TODO
}

inline std::vector<unsigned int>
timeStringsToOffsets(const std::vector<std::string> &timeStrings) {
  std::vector<unsigned int> timeOffsets(timeStrings.size());
  std::transform(
      timeStrings.begin(), timeStrings.end(), timeOffsets.begin(),
      [](const std::string &s) { return timeStringToTimeOffset(s); });
  return timeOffsets;
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
  std::optional<std::vector<std::string>> timeStrings = std::nullopt;
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
    insertIntoParsingInfo<std::vector<std::string>>(yaml,
                                                    parsingInfo.timeStrings);
  } else if (key == TRANSITION) {
    insertIntoParsingInfo<bool>(yaml, parsingInfo.transition);
  } else if (key == TRANSITION_LENGTH) {
    insertIntoParsingInfo<unsigned int>(yaml, parsingInfo.transitionLength);
  } else if (key == TYPE) {
    insertIntoParsingInfo<BackgroundSetType>(yaml, parsingInfo.type);
  }
}

BackgroundSet createBackgroundSetFromInfo(const ParsingInfo &parsingInfo) {
  checkParsingInfoHas(parsingInfo.name, "Background set did not have a name");
  std::string name = parsingInfo.name.value();

  checkParsingInfoHas(
      parsingInfo.type,
      std::format(
          "Background set {} did not specify a type (`type:dynamic/static`)",
          name));

  switch (parsingInfo.type.value()) {
  case BackgroundSetType::Static: {
    checkParsingInfoHasEither(
        parsingInfo.image, parsingInfo.images,
        std::format("Background set {} does not provide an image to show "
                    "(key 'image:' or 'images:')",
                    name));

    checkParsingInfoHas(
        parsingInfo.dataDirectory,
        std::format("background set {} does not provide an image directory "
                    "(key `data_directory:`)",
                    name));

    assert(parsingInfo.image.has_value() || parsingInfo.images.has_value());

    if (parsingInfo.images.has_value()) {
      return BackgroundSet(
          name, StaticBackgroundData(parsingInfo.dataDirectory.value(),
                                     parsingInfo.mode.value_or(DEFAULT_MODE),
                                     parsingInfo.images.value()));
    } else if (parsingInfo.image.has_value()) {
      return BackgroundSet(
          name, StaticBackgroundData(parsingInfo.dataDirectory.value(),
                                     parsingInfo.mode.value_or(DEFAULT_MODE),
                                     {parsingInfo.image.value()}));
    } else {
      throw std::logic_error("Got to end of Static BackgroundSet with both "
                             "image and images unset");
    }
  }
  case BackgroundSetType::Dynamic: {
    checkParsingInfoHas(
        parsingInfo.images,
        std::format("Background set {} has no images (key `images:`)", name));

    checkParsingInfoHas(
        parsingInfo.dataDirectory,
        std::format("Background set {} does not provide an image directory "
                    "(key `data_directory:`)",
                    name));

    checkParsingInfoHas(parsingInfo.timeStrings,
                        std::format("Background set {} does not provide times "
                                    "(key `times:`)",
                                    name));

    std::vector<unsigned int> timeOffsets =
        timeStringsToOffsets(parsingInfo.timeStrings.value());

    return BackgroundSet(
        name, DynamicBackgroundData(parsingInfo.dataDirectory.value(),
                                    parsingInfo.mode.value_or(DEFAULT_MODE),
                                    parsingInfo.transitionLength,
                                    parsingInfo.order.value_or(DEFAULT_ORDER),
                                    parsingInfo.images.value(), timeOffsets));
  }
  default: {
    throw std::logic_error("Unhandled background set type");
  }
  }
}

BackgroundSet parseFromYAML(YAML::const_iterator yaml) {
  ParsingInfo parsingInfo;

  parsingInfo.name = std::make_optional(yaml->first.as<std::string>());

  for (YAML::const_iterator it = yaml->second.begin(); it != yaml->second.end();
       ++it) {
    updateParsingInfoWithYamlNode(it, parsingInfo);
  }

  return createBackgroundSetFromInfo(parsingInfo);
}
} // namespace dynamic_paper
