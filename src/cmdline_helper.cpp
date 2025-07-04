#include "cmdline_helper.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <optional>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>

#include <tl/expected.hpp>
#include <yaml-cpp/node/node.h>

#include "background_set.hpp"
#include "background_set_enums.hpp"
#include "background_setter.hpp"
#include "config.hpp"
#include "constants.hpp"
#include "defaults.hpp"
#include "dynamic_background_set.hpp"
#include "time_from_midnight.hpp"
#include "time_util_current_time.hpp"
#include "variant_visitor_templ.hpp"
#include "yaml_helper.hpp"

namespace dynamic_paper {

// ===== Helper ===================

namespace {

constexpr std::string_view ANSI_BOLD = "\x1b[0m";
constexpr std::string_view ANSI_COLOR_CYAN = "\x1b[36m";
constexpr std::string_view ANSI_COLOR_RED = "\x1b[31m";
constexpr std::string_view ANSI_COLOR_MAGENTA = "\x1b[35m";
constexpr std::string_view ANSI_COLOR_RESET = "\x1b[0m";

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
    logFatalError("Unable to parse background set file {}", backgroundSetFile.string());
    exit(1);
  }

  return yaml.as<std::unordered_map<std::string, YAML::Node>>();
}

/**
 * Prints a relevant error message for `error` caused from parsing `name`
 */
void printParsingError(const std::string &name, const BackgroundSetParseErrors error) {
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
    logError("Unable to parse background {} due to no images being provided", name);
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

void setupLoggingFromYAML(const YAML::Node &config) {
  std::pair<LogLevel, std::filesystem::path> levelAndFileName = loadLoggingInfoFromYAML(config);
  setupLogging(std::move(levelAndFileName));
  logInfo("======== Running dynamic_paper =====");
}

void setupLoggingFromYAMLForStdout(const YAML::Node &config) {
  const std::pair<LogLevel, std::filesystem::path> levelAndFileName =
      loadLoggingInfoFromYAML(config);
  setupLoggingForStdout(levelAndFileName.first);
}

YAML::Node loadConfigFileIntoYAML(const std::filesystem::path &file) {
  if (!std::filesystem::exists(file)) {
    errorMsg("Cannot create config from non-existant file: {}", file.string());
    exit(EXIT_FAILURE);
  }

  try {
    return YAML::LoadFile(file);
  } catch (const YAML::BadFile &e) {
    errorMsg("Could not parse config file `{}`", file.string());
    logError("Could not parse config file {} due to {}", file.string(), e.what());
    exit(EXIT_FAILURE);
  } catch (const YAML::ParserException &e) {
    errorMsg("`{}` config file is malformed.", file.string());
    logError("Could not parse config file {} due to {}", file.string(), e.what());
    exit(EXIT_FAILURE);
  } catch (const std::exception &e) {
    errorMsg("Unable to parse `{}` due to an unknown error: {}", file.string(), e.what());
    logError("Could not parse config file {} due to {}", file.string(), e.what());
    exit(EXIT_FAILURE);
  }
}

Config createConfigFromYAML(const YAML::Node &configYaml, const bool findLocationOverHttp) {
  return loadConfigFromYAML(configYaml, findLocationOverHttp);
}

bool usesInPlaceTransitions(const DynamicBackgroundData &data) {
  return data.transition.has_value() && data.transition->inPlace;
}

void printStaticBackgroundInfo(const StaticBackgroundData &data,
                               const BackgroundSet &backgroundSet) {
  std::cout << ANSI_BOLD << ANSI_COLOR_CYAN << "\n"
            << " ♥ " << backgroundSet.getName() << ANSI_COLOR_RESET << "\n\n";

  std::cout << ANSI_COLOR_MAGENTA << "Mode: " << ANSI_COLOR_RESET
            << backgroundSetModeString(data.mode) << "\n";
  std::cout << ANSI_COLOR_MAGENTA << "Image Directory: " << ANSI_COLOR_RESET << data.imageDirectory
            << "\n";
  if (data.imageNames.size() == 1) {
    std::cout << ANSI_COLOR_MAGENTA << "Image: " << ANSI_COLOR_RESET << data.imageNames.at(0)
              << "\n";
  } else {
    std::cout << ANSI_COLOR_MAGENTA << "Images:\n" << ANSI_COLOR_RESET;
    for (const std::string_view name : data.imageNames) {
      std::cout << " - " << name << "\n";
    }
  }
  std::cout << "\n";
}

void printDynamicBackgroundInfo(const DynamicBackgroundData &data,
                                const BackgroundSet &backgroundSet) {
  std::cout << ANSI_BOLD << ANSI_COLOR_MAGENTA << "\n"
            << " ⭐ " << backgroundSet.getName() << ANSI_COLOR_RESET << "\n\n";

  std::cout << ANSI_COLOR_CYAN << "Mode: " << ANSI_COLOR_RESET << backgroundSetModeString(data.mode)
            << "\n";
  std::cout << ANSI_COLOR_CYAN << "Image Directory: " << ANSI_COLOR_RESET << data.imageDirectory
            << "\n";
  switch (data.order) {
  case BackgroundSetOrder::Linear: {
    std::cout << ANSI_COLOR_CYAN << "Display Order: " << ANSI_COLOR_RESET << "Linear" << "\n";
    break;
  }
  case BackgroundSetOrder::Random: {
    std::cout << ANSI_COLOR_CYAN << "Display Order: " << ANSI_COLOR_RESET << "Random" << "\n";
    break;
  }
  }

  switch (data.order) {
  case BackgroundSetOrder::Linear: {
    std::cout << ANSI_COLOR_CYAN << "Images:" << ANSI_COLOR_RESET << "\n";

    for (int i = 0; std::cmp_less(i, std::min(data.imageNames.size(), data.times.size())); i++) {
      std::cout << " - " << data.imageNames.at(i) << " -> " << ANSI_COLOR_MAGENTA
                << data.times.at(i) << ANSI_COLOR_RESET << "\n";
    }
    break;
  }
  case BackgroundSetOrder::Random: {
    std::cout << ANSI_COLOR_CYAN << "Images: " << ANSI_COLOR_RESET << "\n";
    for (const std::string_view name : data.imageNames) {
      std::cout << " - " << name << "\n";
    }

    std::cout << ANSI_COLOR_CYAN << "Times: " << ANSI_COLOR_RESET << "\n";
    for (const TimeFromMidnight &time : data.times) {
      std::cout << " - " << time << "\n";
    }
    break;
  }
  }
  std::cout << "\n";
}

bool shouldUseScriptToSetBackground(const Config &config) {
  return std::visit(
      overloaded{
          [](const MethodWallUtils /* method */) { return false; },
          [](const std::filesystem::path & /* path */) { return true; },
      },
      config.method);
}

auto backgroundSetterScriptFunc(const Config &config) {
  const std::filesystem::path *script =
      std::get_if<std::filesystem::path>(&config.method);
  if (script == nullptr) {
    throw std::logic_error("Tried to get a background setter function when the "
                           "config method is not a script");
  }

  return [script](const std::filesystem::path &imagePath,
                  const BackgroundSetMode mode) {
    setBackgroundToImageUsingScript(*script, imagePath, mode);
  };
}

} // namespace

// ===== Header ====================

Config getConfigAndSetupLogging(const argparse::ArgumentParser &program,
                                const bool findLocationOverHttp) {
  const std::filesystem::path conf = program.get(CONFIG_FLAG_NAME);
  const bool logToStdout = program.get<bool>(LOG_TO_STDOUT_FLAG_NAME);

  // TODO not taking this value?
  auto configFilePath = std::filesystem::path(expandPath(conf));

  if (configFilePath == expandPath(DEFAULT_CONFIG_FILE_NAME)) {
    const bool fileCreationResult =
        FilesystemHandler::createFileIfDoesntExist(configFilePath, DEFAULT_CONFIG_FILE_CONTENTS);

    if (!fileCreationResult) {
      errorMsg("Error creating default config file: {}", configFilePath.string());
      exit(EXIT_FAILURE);
    }
  }

  const YAML::Node configYaml = loadConfigFileIntoYAML(configFilePath);

  if (logToStdout) {
    setupLoggingFromYAMLForStdout(configYaml);
  } else {
    setupLoggingFromYAML(configYaml);
  }

  return createConfigFromYAML(configYaml, findLocationOverHttp);
}

void showCacheInfo(const Config &config) {
  constexpr std::string_view ANSI_COLOR_CYAN = "\x1b[36m";
  constexpr std::string_view ANSI_COLOR_RESET = "\x1b[0m";

  std::cout << "Cache files are stored in " << ANSI_COLOR_CYAN
            << config.imageCacheDirectory.string() << ANSI_COLOR_RESET << "\n";
}

bool isBeingPiped() { return isatty(fileno(stdin)) == 0; }

/**
 * Parses `BackgroundSet`s from the config file, exiting the program if unable
 * to parse one
 */
std::vector<BackgroundSet> getBackgroundSetsFromFile(const Config &config) {
  const std::unordered_map<std::string, YAML::Node> yamlMap =
      nameAndYAMLInfoFromFile(config.backgroundSetConfigFile);

  std::vector<BackgroundSet> backgroundSets;
  backgroundSets.reserve(yamlMap.size());

  const SolarDay solarDay = config.solarDayProvider.getSolarDay();

  for (const auto &keyValue : yamlMap) {
    tl::expected<BackgroundSet, BackgroundSetParseErrors> expBackgroundSet =
        parseFromYAML(keyValue.first, keyValue.second, solarDay);

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

std::vector<std::pair<std::string_view, BackgroundSetType>>
getNamesAndTypes(const std::vector<BackgroundSet> &backgroundSets) {
  std::vector<std::pair<std::string_view, BackgroundSetType>> namesAndTypes;
  namesAndTypes.reserve(backgroundSets.size());
  std::ranges::transform(
      backgroundSets, std::back_inserter(namesAndTypes),
      [](const BackgroundSet &set) { return std::make_pair(set.getName(), set.getType()); });
  std::ranges::sort(namesAndTypes, {}, &std::pair<std::string_view, BackgroundSetType>::first);
  return namesAndTypes;
}

std::optional<BackgroundSet> getBackgroundSetWithNameFromFile(const std::string_view name,
                                                              const Config &config) {
  const std::unordered_map<std::string, YAML::Node> yamlMap =
      nameAndYAMLInfoFromFile(config.backgroundSetConfigFile);

  for (const auto &keyValue : yamlMap) {
    if (keyValue.first == name) {
      tl::expected<BackgroundSet, BackgroundSetParseErrors> expBackgroundSet =
          parseFromYAML(keyValue.first, keyValue.second, config.solarDayProvider.getSolarDay());

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
        parseFromYAML(name, yamlNode, config.solarDayProvider.getSolarDay());

    if (expBackgroundSet.has_value()) {
      logDebug("success; returning {}", expBackgroundSet->getName());
      return expBackgroundSet.value();
    }

    printParsingError(name, expBackgroundSet.error());
  }

  return std::nullopt;
}

//  std::optional<std::tuple<std::filesystem::path, BackgroundSetMode >>
std::optional<std::pair<std::filesystem::path, BackgroundSetMode>>
getRandomImageAndModeFromAllBackgroundSets(const Config &config) {
  const std::unordered_map<std::string, YAML::Node> yamlMap =
      nameAndYAMLInfoFromFile(config.backgroundSetConfigFile);

  std::vector<std::pair<std::filesystem::path, BackgroundSetMode>> wallpaperOptions;

  const auto validNode = [](const std::pair<std::string, YAML::Node> &nameAndNode) {
    const YAML::Node &node = nameAndNode.second;
    return node.IsDefined() && node[MODE].IsDefined() && node[IMAGE_DIRECTORY].IsDefined() &&
           ((node[IMAGES].IsDefined() && node[IMAGES].IsSequence()) || (node[IMAGE].IsDefined()));
  };

  for (const auto &key : yamlMap | std::ranges::views::filter(validNode)) {
    const YAML::Node &node = key.second;

    const BackgroundSetMode mode =
        yamlStringTo<BackgroundSetMode>(node[MODE].as<std::string>()).value();

    const std::filesystem::path imageDirectory = expandPath(
        yamlStringTo<std::filesystem::path>(node[IMAGE_DIRECTORY].as<std::string>()).value());

    if (node[IMAGES].IsDefined()) {
      for (const auto &nodeImage : node[IMAGES]) {
        const auto imagePath = yamlStringTo<std::filesystem::path>(nodeImage.as<std::string>());
        if (!imagePath) {
          continue;
        }
        wallpaperOptions.emplace_back(imageDirectory / imagePath.value(), mode);
      }
    } else if (node[IMAGE].IsDefined()) {
      const auto imagePath = yamlStringTo<std::filesystem::path>(node[IMAGE].as<std::string>());
      if (!imagePath) {
        continue;
      }
      wallpaperOptions.emplace_back(imageDirectory / imagePath.value(), mode);
    } else {
      continue;
    }
  }

  if (wallpaperOptions.empty()) {
    return std::nullopt;
  }

  std::random_device randomDevice;
  std::mt19937 generator(randomDevice());
  std::uniform_int_distribution<> distribution(0, static_cast<int>(wallpaperOptions.size() - 1));
  const std::size_t index = distribution(generator);

  return wallpaperOptions.at(index);
}

void showBackgroundSet(BackgroundSet &backgroundSet, const Config &config) {
  std::cout << "Showing: " << backgroundSet.getName() << '\n';

  std::optional<StaticBackgroundData> staticData = backgroundSet.getStaticBackgroundData();

  if (staticData.has_value()) {
    if (shouldUseScriptToSetBackground(config)) {
      const auto backgroundSetterFunc = backgroundSetterScriptFunc(config);
      staticData->show(config, backgroundSetterFunc);
    } else {
      staticData->show(config, setBackgroundToImage);
    }
  }

  std::optional<DynamicBackgroundData> dynamicData = backgroundSet.getDynamicBackgroundData();
  if (dynamicData.has_value()) {
    while (true) {
      const TimeFromMidnight currentTime = getCurrentTime();
      logDebug("Current time is {}", currentTime);

      std::chrono::seconds sleepTime{};

      if (usesInPlaceTransitions(dynamicData.value())) {
        if (shouldUseScriptToSetBackground(config)) {
          const auto backgroundSetterFunc = backgroundSetterScriptFunc(config);
          sleepTime =
              dynamicData->updateBackground<decltype(backgroundSetterFunc),
                                     FilesystemHandler, ImageCompositorInPlace>(
                      currentTime, config, std::move(backgroundSetterFunc)) +
              std::chrono::seconds(1);
        } else {
          sleepTime =
              dynamicData
                  ->updateBackground<decltype(&setBackgroundToImage),
                                     FilesystemHandler, ImageCompositorInPlace>(
                      currentTime, config, &setBackgroundToImage) +
              std::chrono::seconds(1);
        }
      } else {
        if (shouldUseScriptToSetBackground(config)) {
          const auto backgroundSetterFunc = backgroundSetterScriptFunc(config);
          sleepTime = dynamicData->updateBackground(currentTime, config,
                                                    backgroundSetterFunc) +
                      std::chrono::seconds(1);
        } else {
          sleepTime = dynamicData->updateBackground(currentTime, config,
                                                    &setBackgroundToImage) +
                      std::chrono::seconds(1);
        }
      }

      logDebug("Sleeping for {} seconds...", sleepTime);
      flushLogger();
      std::this_thread::sleep_for(sleepTime);
    }
  }
}

void printBackgroundSetInfo(const BackgroundSet &backgroundSet) {
  std::optional<StaticBackgroundData> staticData = backgroundSet.getStaticBackgroundData();
  if (staticData.has_value()) {
    printStaticBackgroundInfo(staticData.value(), backgroundSet);
  }

  const std::optional<DynamicBackgroundData> dynamicData = backgroundSet.getDynamicBackgroundData();
  if (dynamicData.has_value()) {
    printDynamicBackgroundInfo(dynamicData.value(), backgroundSet);
  }
}

} // namespace dynamic_paper
