#include <format>
#include <iostream>

#include <argparse/argparse.hpp>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include "background_set.hpp"
#include "config.hpp"
#include "logger.hpp"

// ===== Background Set ==========

/**
 * Prints a relevant error message for `error` caused from parsing `name`
 * */
[[noreturn]] static void
exitAndPrintFatalError(const std::string &name,
                       const dynamic_paper::BackgroundSetParseErrors error) {
  switch (error) {
  case dynamic_paper::BackgroundSetParseErrors::MissingSunpollInfo: {
    dynamic_paper::logFatalError(
        std::format("Unable to parse background {} due to not being able "
                    "to determine time of sunrise and sunset",
                    name));
    break;
  }
  case dynamic_paper::BackgroundSetParseErrors::BadTimes: {
    dynamic_paper::logFatalError(
        std::format("Unable to parse background {} due to bad times", name));
    break;
  }
  case dynamic_paper::BackgroundSetParseErrors::NoTimes: {
    dynamic_paper::logFatalError(
        std::format("Unable to parse background {} due to no times to "
                    "transition being provided",
                    name));
    break;
  }
  case dynamic_paper::BackgroundSetParseErrors::NoImages: {
    dynamic_paper::logFatalError(std::format(
        "Unable to parse background {} due to no images being provided", name));
    break;
  }
  case dynamic_paper::BackgroundSetParseErrors::NoImageDirectory: {
    dynamic_paper::logFatalError(
        std::format("Unable to parse background {} due to no image data "
                    "directory provided",
                    name));
    break;
  }
  case dynamic_paper::BackgroundSetParseErrors::NoName: {
    dynamic_paper::logFatalError(std::format(
        "Unable to parse background {} due to no name provided", name));
    break;
  }
  case dynamic_paper::BackgroundSetParseErrors::NoType: {
    dynamic_paper::logFatalError(std::format(
        "Unable to parse background {} due to no type provided", name));
    break;
  }
  }

  exit(1);
}

void setupLogging(const YAML::Node &config) {
  dynamic_paper::LogLevel logLevel =
      dynamic_paper::loadLoggingLevelFromYAML(config);
  setupLogging(logLevel);
}

/**
 * Parses `BackgroundSet`s from the config file, exiting the program if unable
 * to parse one
 */
static std::vector<dynamic_paper::BackgroundSet>
getBackgroundSetsFromFile(const std::filesystem::path backgroundSetFile,
                          const dynamic_paper::Config &config) {
  YAML::Node yaml;

  try {
    yaml = YAML::LoadFile(backgroundSetFile);
  } catch (const YAML::ParserException &e) {
    dynamic_paper::logFatalError("Unable to parse background set file " +
                                 backgroundSetFile.string());
    exit(1);
  }

  auto yamlMap = yaml.as<std::unordered_map<std::string, YAML::Node>>();

  std::vector<dynamic_paper::BackgroundSet> backgroundSets;
  backgroundSets.reserve(yamlMap.size());

  for (const auto &kv : yamlMap) {
    std::expected<dynamic_paper::BackgroundSet,
                  dynamic_paper::BackgroundSetParseErrors>
        expBackgroundSet = parseFromYAML(kv.first, kv.second, config);

    if (expBackgroundSet.has_value()) {
      const dynamic_paper::BackgroundSet &backgroundSet =
          expBackgroundSet.value();
      backgroundSets.push_back(backgroundSet);
      dynamic_paper::logInfo("Added background: " + backgroundSet.name);
    } else {
      exitAndPrintFatalError(kv.first, expBackgroundSet.error());
    }
  }

  return backgroundSets;
}

// ===== Command Line Parsing ==========
static void handleShowCommand(argparse::ArgumentParser &showCommand) {
  std::string wallpaperName = showCommand.get("name");
  std::cout << wallpaperName << std::endl;
}

static void handleRandomCommand() { std::cout << "random" << std::endl; }

static void handleListCommand(const dynamic_paper::Config &config) {
  std::vector<dynamic_paper::BackgroundSet> backgroundSets =
      getBackgroundSetsFromFile("./files/background_sets.yaml", config);
  std::cout << "Available Background sets are: "
            << "\n"
            << std::endl;
  for (const auto &b : backgroundSets) {
    std::cout << b.name << std::endl;
  }
}

static bool parseArguements(const int argc, char *argv[],
                            const dynamic_paper::Config &config) {
  argparse::ArgumentParser program("dynamic paper");

  argparse::ArgumentParser showCommand("show");
  showCommand.add_description("Show wallpaper set with name");
  showCommand.add_argument("name").help("Name of wallpaper set to show");

  argparse::ArgumentParser randomCommand("random");
  randomCommand.add_description("Show a random wallpaper set");

  argparse::ArgumentParser listCommand("list");
  listCommand.add_description("List all wallpaper set options");

  program.add_subparser(showCommand);
  program.add_subparser(randomCommand);
  program.add_subparser(listCommand);

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error &err) {
    std::cout << "Error parsing command line options:" << std::endl;
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return false;
  }

  if (program.is_subcommand_used(showCommand)) {
    handleShowCommand(showCommand);
  } else if (program.is_subcommand_used(listCommand)) {
    handleListCommand(config);
  } else if (program.is_subcommand_used(randomCommand)) {
    handleRandomCommand();
  }

  return true;
}

auto main(int argc, char *argv[]) -> int {
  dynamic_paper::Config config =
      dynamic_paper::loadConfigFromYAML(YAML::LoadFile("./files/test.yaml"))
          .value();

  bool result = parseArguements(argc, argv, config);

  return result ? 0 : 1;
}
