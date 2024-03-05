#include "cmdline_helper.hpp"

#include <random>
#include <ranges>

#include "background_set.hpp"
#include "logger.hpp"

namespace dynamic_paper {

// ===== Helper ===================

namespace {

/**
 *  Parses yaml info in `backgroundSetFile` into a pair that maps the name of
 * each `BackgroundSet` to YAML info that describes it
 */
std::unordered_map<std::string, YAML::Node>
nameAndYAMLInfoFromFile(const std::filesystem::path &backgroundSetFile) {
  YAML::Node yaml;

  try {
    yaml = YAML::LoadFile(backgroundSetFile);
  } catch (const YAML::ParserException &e) {
    logFatalError("Unable to parse background set file {}",
                  backgroundSetFile.string());
    exit(1);
  }

  return yaml.as<std::unordered_map<std::string, YAML::Node>>();
}

/**
 * Prints a relevant error message for `error` caused from parsing `name`
 */
void printParsingError(const std::string &name,
                       const BackgroundSetParseErrors error) {
  switch (error) {
  case BackgroundSetParseErrors::MissingSunpollInfo: {
    logError("Unable to parse background {} due to not being able "
             "to determine time of sunrise and sunset",
             name);
    break;
  }
  case BackgroundSetParseErrors::BadTimes: {
    logError("Unable to parse background {} due to bad times", name);
    break;
  }
  case BackgroundSetParseErrors::NoTimes: {
    logError("Unable to parse background {} due to no times to "
             "transition being provided",
             name);
    break;
  }
  case BackgroundSetParseErrors::NoImages: {
    logError("Unable to parse background {} due to no images being provided",
             name);
    break;
  }
  case BackgroundSetParseErrors::NoImageDirectory: {
    logError("Unable to parse background {} due to no image data "
             "directory provided",
             name);
    break;
  }
  case BackgroundSetParseErrors::NoName: {
    logError("Unable to parse background {} due to no name provided", name);
    break;
  }
  case BackgroundSetParseErrors::NoType: {
    logError("Unable to parse background {} due to no type provided", name);
    break;
  }
  }
}

} // namespace

// ===== Header ====================

/**
 * Parses `BackgroundSet`s from the config file, exiting the program if unable
 * to parse one
 */
std::vector<BackgroundSet> getBackgroundSetsFromFile(const Config &config) {
  const std::unordered_map<std::string, YAML::Node> yamlMap =
      nameAndYAMLInfoFromFile(config.backgroundSetConfigFile);

  std::vector<BackgroundSet> backgroundSets;
  backgroundSets.reserve(yamlMap.size());

  for (const auto &keyValue : yamlMap) {
    tl::expected<BackgroundSet, BackgroundSetParseErrors> expBackgroundSet =
        parseFromYAML(keyValue.first, keyValue.second, config);

    if (expBackgroundSet.has_value()) {
      const BackgroundSet &backgroundSet = expBackgroundSet.value();
      backgroundSets.push_back(backgroundSet);
      logInfo("Added background: {}", backgroundSet.getName());
    } else {
      printParsingError(keyValue.first, expBackgroundSet.error());
    }
  }

  return backgroundSets;
}

std::optional<BackgroundSet>
getBackgroundSetWithNameFromFile(const std::string_view name,
                                 const Config &config) {
  const std::unordered_map<std::string, YAML::Node> yamlMap =
      nameAndYAMLInfoFromFile(config.backgroundSetConfigFile);

  for (const auto &keyValue : yamlMap) {
    if (keyValue.first == name) {
      tl::expected<BackgroundSet, BackgroundSetParseErrors> expBackgroundSet =
          parseFromYAML(keyValue.first, keyValue.second, config);

      if (expBackgroundSet.has_value()) {
        return expBackgroundSet.value();
      }

      printParsingError(keyValue.first, expBackgroundSet.error());
    }
  }

  return std::nullopt;
}

std::optional<BackgroundSet> getRandomBackgroundSet(const Config &config) {
  std::unordered_map<std::string, YAML::Node> yamlMap =
      nameAndYAMLInfoFromFile(config.backgroundSetConfigFile);

  // NOTE: YAML::Node does not std::shuffle or copy properly so have to use
  // alternate method keeping it in the unordered map

  std::vector<std::string> yamlMapKeys;
  yamlMapKeys.reserve(yamlMap.size());
  for (const auto &key : yamlMap) {
    yamlMapKeys.push_back(key.first);
  }

  std::vector<size_t> indeces;
  indeces.reserve(yamlMap.size());
  for (size_t i = 0; i < yamlMap.size(); i++) {
    indeces.push_back(i);
  }

  std::random_device randomDevice;
  std::mt19937 generator(randomDevice());
  std::shuffle(indeces.begin(), indeces.end(), generator);

  for (const size_t index : indeces) {
    const std::string &name = yamlMapKeys.at(index);
    const auto &yamlNode = yamlMap.at(name);

    tl::expected<BackgroundSet, BackgroundSetParseErrors> expBackgroundSet =
        parseFromYAML(name, yamlNode, config);

    if (expBackgroundSet.has_value()) {
      logDebug("success; returning {}", expBackgroundSet->getName());
      return expBackgroundSet.value();
    }

    printParsingError(name, expBackgroundSet.error());
  }

  return std::nullopt;
}

} // namespace dynamic_paper
