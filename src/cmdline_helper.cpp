#include "cmdline_helper.hpp"

#include <random>
#include <ranges>

#include "background_set.hpp"
#include "logger.hpp"

namespace dynamic_paper {

// ===== Helper ===================

constexpr std::string_view ANSI_COLOR_RED = "\x1b[31m";
constexpr std::string_view ANSI_COLOR_GREEN = "\x1b[32m";
constexpr std::string_view ANSI_COLOR_YELLOW = "\x1b[33m";
constexpr std::string_view ANSI_COLOR_BLUE = "\x1b[34m";
constexpr std::string_view ANSI_COLOR_MAGENTA = "\x1b[35m";
constexpr std::string_view ANSI_COLOR_CYAN = "\x1b[36m";
constexpr std::string_view ANSI_COLOR_RESET = "\x1b[0m";

/**
 *  Parses yaml info in `backgroundSetFile` into a pair that maps the name of
 * each `BackgroundSet` to YAML info that describes it
 */
static std::unordered_map<std::string, YAML::Node>
nameAndYAMLInfoFromFile(const std::filesystem::path &backgroundSetFile) {
  YAML::Node yaml;

  try {
    yaml = YAML::LoadFile(backgroundSetFile);
  } catch (const YAML::ParserException &e) {
    logFatalError("Unable to parse background set file " +
                  backgroundSetFile.string());
    exit(1);
  }

  return yaml.as<std::unordered_map<std::string, YAML::Node>>();
}

/**
 * Prints a relevant error message for `error` caused from parsing `name`
 */
static void printParsingError(const std::string &name,
                              const BackgroundSetParseErrors error) {
  switch (error) {
  case BackgroundSetParseErrors::MissingSunpollInfo: {
    logError(std::format("Unable to parse background {} due to not being able "
                         "to determine time of sunrise and sunset",
                         name));
    break;
  }
  case BackgroundSetParseErrors::BadTimes: {
    logError(
        std::format("Unable to parse background {} due to bad times", name));
    break;
  }
  case BackgroundSetParseErrors::NoTimes: {
    logError(std::format("Unable to parse background {} due to no times to "
                         "transition being provided",
                         name));
    break;
  }
  case BackgroundSetParseErrors::NoImages: {
    logError(std::format(
        "Unable to parse background {} due to no images being provided", name));
    break;
  }
  case BackgroundSetParseErrors::NoImageDirectory: {
    logError(std::format("Unable to parse background {} due to no image data "
                         "directory provided",
                         name));
    break;
  }
  case BackgroundSetParseErrors::NoName: {
    logError(std::format(
        "Unable to parse background {} due to no name provided", name));
    break;
  }
  case BackgroundSetParseErrors::NoType: {
    logError(std::format(
        "Unable to parse background {} due to no type provided", name));
    break;
  }
  }
}

// ===== Header ====================

/**
 * Parses `BackgroundSet`s from the config file, exiting the program if unable
 * to parse one
 */
static std::vector<BackgroundSet>
getBackgroundSetsFromFile(const std::filesystem::path backgroundSetFile,
                          const Config &config) {
  std::unordered_map<std::string, YAML::Node> yamlMap =
      nameAndYAMLInfoFromFile(backgroundSetFile);

  std::vector<BackgroundSet> backgroundSets;
  backgroundSets.reserve(yamlMap.size());

  for (const auto &kv : yamlMap) {
    std::expected<BackgroundSet, BackgroundSetParseErrors> expBackgroundSet =
        parseFromYAML(kv.first, kv.second, config);

    if (expBackgroundSet.has_value()) {
      const BackgroundSet &backgroundSet = expBackgroundSet.value();
      backgroundSets.push_back(backgroundSet);
      logInfo("Added background: " + backgroundSet.name);
    } else {
      printParsingError(kv.first, expBackgroundSet.error());
    }
  }

  return backgroundSets;
}

std::optional<BackgroundSet>
getBackgroundSetWithNameFromFile(const std::string_view name,
                                 const std::filesystem::path backgroundSetFile,
                                 const Config &config) {
  std::unordered_map<std::string, YAML::Node> yamlMap =
      nameAndYAMLInfoFromFile(backgroundSetFile);

  for (const auto &kv : yamlMap) {
    if (kv.first == name) {
      std::expected<BackgroundSet, BackgroundSetParseErrors> expBackgroundSet =
          parseFromYAML(kv.first, kv.second, config);

      if (expBackgroundSet.has_value()) {
        return expBackgroundSet.value();
      } else {
        printParsingError(kv.first, expBackgroundSet.error());
      }
    }
  }

  return std::nullopt;
}

std::optional<BackgroundSet>
getRandomBackgroundSet(const std::filesystem::path backgroundSetFile,
                       const Config &config) {
  std::unordered_map<std::string, YAML::Node> yamlMap =
      nameAndYAMLInfoFromFile(backgroundSetFile);

  std::vector<std::pair<std::string, YAML::Node>> yamlPairs;
  std::ranges::copy(yamlMap.begin(), yamlMap.end(),
                    std::back_inserter(yamlPairs));

  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(yamlPairs.begin(), yamlPairs.end(), g);

  for (const auto &nameYaml : yamlPairs) {
    std::expected<BackgroundSet, BackgroundSetParseErrors> expBackgroundSet =
        parseFromYAML(nameYaml.first, nameYaml.second, config);

    if (expBackgroundSet.has_value()) {
      return expBackgroundSet.value();
    } else {
      printParsingError(nameYaml.first, expBackgroundSet.error()) :
    }
  }

  return std::nullopt;
}

} // namespace dynamic_paper
