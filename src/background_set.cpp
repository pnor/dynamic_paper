#include "background_set.hpp"

#include <algorithm>
#include <cassert>
#include <format>
#include <type_traits>

#include "time_util.hpp"
#include "type_helper.hpp"
#include "yaml_helper.hpp"

namespace dynamic_paper {

// YAML Dict Key
static constexpr std::string_view DATA_DIRECTORY = "data_directory";
static constexpr std::string_view IMAGES = "images";
static constexpr std::string_view IMAGE = "image";
static constexpr std::string_view MODE = "mode";
static constexpr std::string_view ORDER = "order";
static constexpr std::string_view TIMES = "times";
static constexpr std::string_view TRANSITION_LENGTH = "transition_length";
static constexpr std::string_view TYPE = "type";
// Default value
static constexpr BackgroundSetMode DEFAULT_MODE = BackgroundSetMode::Center;
static constexpr BackgroundSetOrder DEFAULT_ORDER = BackgroundSetOrder::Linear;

// ===== Helper ===============

template <typename T>
static void insertIntoParsingInfo(const YAML::Node &yaml,
                                  std::optional<T> &field) {
  if constexpr (std::is_same_v<T, std::string>) {
    T yamlData = yaml.as<T>();
    field = std::make_optional(yamlData);
  } else if constexpr (is_vector<T>::value) {
    T yamlData;
    yamlData.reserve(yaml.size());

    for (std::size_t i = 0; i < yaml.size(); i++) {
      yamlData.push_back(yaml[i].as<typename T::value_type>());
    }

    field = std::make_optional(yamlData);
  } else {
    std::optional<T> optData = stringTo<T>(yaml.as<std::string>());
    if (optData.has_value()) {
      field = optData;
    }
  }
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

// ===== Constructor ===============

StaticBackgroundData::StaticBackgroundData(std::filesystem::path dataDirectory,
                                           BackgroundSetMode mode,
                                           std::vector<std::string> imageNames)
    : dataDirectory(dataDirectory), mode(mode), imageNames(imageNames) {}

DynamicBackgroundData::DynamicBackgroundData(
    std::filesystem::path dataDirectory, BackgroundSetMode mode,
    std::optional<unsigned int> transitionDuration, BackgroundSetOrder order,
    std::vector<std::string> imageNames, std::vector<time_t> times)
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
  std::optional<unsigned int> transitionLength = std::nullopt;
  std::optional<BackgroundSetOrder> order = std::nullopt;
  std::optional<std::vector<std::string>> images = std::nullopt;
  std::optional<std::string> image = std::nullopt;
  std::optional<std::vector<std::string>> timeStrings = std::nullopt;
};

static void updateParsingInfoWithYamlNode(const std::string &key,
                                          const YAML::Node &value,
                                          ParsingInfo &parsingInfo) {
  if (key == DATA_DIRECTORY && value.IsScalar()) {
    insertIntoParsingInfo<std::filesystem::path>(value,
                                                 parsingInfo.dataDirectory);
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
  }
}

static std::expected<BackgroundSet, BackgroundSetParseErrors>
createStaticBackgroundSetFromInfo(const ParsingInfo &parsingInfo,
                                  const Config &config) {
  assert((void("Parsing info should have name by time this helper function "
               "is called"),
          parsingInfo.name.has_value()));

  const std::string &name = parsingInfo.name.value();

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

  assert((void("Parsing info must have an image"),
          parsingInfo.image.has_value() || parsingInfo.images.has_value()));

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

static std::expected<BackgroundSet, BackgroundSetParseErrors>
createDynamicBackgroundSetFromInfo(const ParsingInfo &parsingInfo,
                                   const Config &config) {
  assert((void("Parsing info should have name by time this helper function "
               "is called"),
          parsingInfo.name.has_value()));

  const std::string &name = parsingInfo.name.value();

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

  std::optional<SunriseAndSunsetTimes> optSunriseAndSunsetTimes =
      getSunriseAndSetString(config);
  if (!optSunriseAndSunsetTimes.has_value()) {
    return std::unexpected(BackgroundSetParseErrors::MissingSunpollInfo);
  }

  std::optional<std::vector<time_t>> optTimeOffsets = timeStringsToTimes(
      parsingInfo.timeStrings.value(), optSunriseAndSunsetTimes.value());
  if (!optTimeOffsets.has_value()) {
    return std::unexpected(BackgroundSetParseErrors::BadTimes);
  }

  return BackgroundSet(
      name, DynamicBackgroundData(parsingInfo.dataDirectory.value(),
                                  parsingInfo.mode.value_or(DEFAULT_MODE),
                                  parsingInfo.transitionLength,
                                  parsingInfo.order.value_or(DEFAULT_ORDER),
                                  parsingInfo.images.value(),
                                  optTimeOffsets.value()));
}

static std::expected<BackgroundSet, BackgroundSetParseErrors>
createBackgroundSetFromInfo(const ParsingInfo &parsingInfo,
                            const Config &config) {
  checkParsingInfoHas(parsingInfo.name, "Background set did not have a name");
  std::string name = parsingInfo.name.value();

  checkParsingInfoHas(
      parsingInfo.type,
      std::format(
          "Background set {} did not specify a type (`type:dynamic/static`)",
          name));

  switch (parsingInfo.type.value()) {
  case BackgroundSetType::Static: {
    return createStaticBackgroundSetFromInfo(parsingInfo, config);
  }
  case BackgroundSetType::Dynamic: {
    return createDynamicBackgroundSetFromInfo(parsingInfo, config);
  }
  default: {
    throw std::logic_error("Unhandled background set type");
  }
  }
}

std::expected<BackgroundSet, BackgroundSetParseErrors>
parseFromYAML(const std::string &name, const YAML::Node &yaml,
              const Config &config) {
  ParsingInfo parsingInfo;

  std::unordered_map<std::string, YAML::Node> yamlMap =
      yaml.as<std::unordered_map<std::string, YAML::Node>>();

  parsingInfo.name = std::make_optional(name);

  for (const auto &kv : yamlMap) {
    updateParsingInfoWithYamlNode(kv.first, kv.second, parsingInfo);
  }

  return createBackgroundSetFromInfo(parsingInfo, config);
}
} // namespace dynamic_paper