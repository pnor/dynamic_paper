#include <format>
#include <iostream>

#include <argparse/argparse.hpp>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>
#include <tl/expected.hpp>
#include <yaml-cpp/yaml.h>

#include "background_set.hpp"
#include "cmdline_helper.hpp"
#include "config.hpp"
#include "defaults.hpp"
#include "logger.hpp"

using namespace dynamic_paper;

// TODO try and stop using less shell commands? (exec family maybe)
// - libgeoclue for location
// - buelowp sunset
// TODO better README
// TODO config to just manually choose sunrise and sunset
// TODO rehaul general config for cmdline options, then file then defualts?
// TODO log to stdout option
// TODO cache management (delete all and for one set + show location)
// TODO high level defautl config options for background_sets.yaml (example:
// specify default transition for all)

namespace {

constexpr std::string_view ANSI_COLOR_CYAN = "\x1b[36m";
constexpr std::string_view ANSI_COLOR_RESET = "\x1b[0m";

// ===== Command Line Arguements ===============

void handleShowCommand(argparse::ArgumentParser &showCommand,
                       const Config &config) {
  const std::string &name = showCommand.get("name");

  std::optional<BackgroundSet> optBackgroundSet =
      getBackgroundSetWithNameFromFile(name, config);

  if (!optBackgroundSet.has_value()) {
    std::cout << "Unable to show background set with name " << name << '\n';
    return;
  }

  showBackgroundSet(optBackgroundSet.value(), config);
}

void handleRandomCommand(const Config &config) {
  std::optional<BackgroundSet> optBackgroundSet =
      getRandomBackgroundSet(config);

  if (!optBackgroundSet.has_value()) {
    std::cout << "Unable to parse any background set from the config file at : "
              << config.backgroundSetConfigFile.string() << std::endl;
    return;
  }

  logDebug("Showing background set: {}", optBackgroundSet->getName());

  showBackgroundSet(optBackgroundSet.value(), config);
}

void handleListCommand(const Config &config) {
  if (!std::filesystem::exists(config.backgroundSetConfigFile)) {
    errorMsg("No config file exists for background sets at path: {}",
             config.backgroundSetConfigFile.string());
    exit(EXIT_FAILURE);
  }

  const std::vector<BackgroundSet> backgroundSets =
      getBackgroundSetsFromFile(config);

  const std::vector<std::pair<std::string_view, BackgroundSetType>>
      namesAndTypes = getNamesAndTypes(backgroundSets);

  if (isBeingPiped()) {
    for (const auto &nameType : namesAndTypes) {
      std::cout << nameType.first << "\n";
    }
  } else {
    std::cout << "Available Background Sets are:\n";

    for (const auto &nameType : namesAndTypes) {
      switch (nameType.second) {
      case BackgroundSetType::Static: {
        std::cout << "- " << nameType.first << "\n";
        break;
      }
      case BackgroundSetType::Dynamic: {
        std::cout << "- " << ANSI_COLOR_CYAN << nameType.first
                  << ANSI_COLOR_RESET << "\n";
        break;
      }
      }
    }
    std::cout << "\n";
  }
}

void handleCacheCommand(const Config &config,
                        const argparse::ArgumentParser &cache,
                        const argparse::ArgumentParser &info) {
  if (cache.is_subcommand_used(info)) {
    showCacheInfo(config);
  } else {
    errorMsg("No subcommand was chosen");
    std::cout << cache.help().str();
  }
}

void showHelp(const argparse::ArgumentParser &program) {
  std::cout << program.help().str();
}

} // namespace

// ===== Main ===============

auto main(int argc, char *argv[]) -> int {
  argparse::ArgumentParser program("dynamic_paper");
  program.add_argument("--config")
      .default_value<std::string>(std::string(DEFAULT_CONFIG_FILE_NAME))
      .required()
      .help("Show optional config");

  argparse::ArgumentParser showCommand("show");
  showCommand.add_description("Show wallpaper set with name");
  showCommand.add_argument("name").help("Name of wallpaper set to show");

  argparse::ArgumentParser randomCommand("random");
  randomCommand.add_description("Show a random wallpaper set");

  argparse::ArgumentParser listCommand("list");
  listCommand.add_description("List all wallpaper set options");

  argparse::ArgumentParser helpCommand("help");
  listCommand.add_description("Show help");

  argparse::ArgumentParser cacheCommand("cache");
  cacheCommand.add_description("Manage cache for interpolated images");
  argparse::ArgumentParser cacheInfoCommand("info");
  cacheInfoCommand.add_description(
      "Show information about cached interpolated images");
  cacheCommand.add_subparser(cacheInfoCommand);

  program.add_subparser(showCommand);
  program.add_subparser(randomCommand);
  program.add_subparser(listCommand);
  program.add_subparser(helpCommand);
  program.add_subparser(cacheCommand);

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error &err) {
    errorMsg("Error parsing command line options:\n{}", err.what());
    return EXIT_FAILURE;
  } catch (const std::exception &generalException) {
    errorMsg("An unknown error occurred!\n{}", generalException.what());
  }

  if (program.is_subcommand_used(showCommand)) {
    const Config config = getConfigAndSetupLogging(program);
    handleShowCommand(showCommand, config);
  } else if (program.is_subcommand_used(listCommand)) {
    const Config config = getConfigAndSetupLogging(program);
    handleListCommand(config);
  } else if (program.is_subcommand_used(randomCommand)) {
    const Config config = getConfigAndSetupLogging(program);
    handleRandomCommand(config);
  } else if (program.is_subcommand_used(cacheCommand)) {
    const Config config = getConfigAndSetupLogging(program);
    handleCacheCommand(config, cacheCommand, cacheInfoCommand);
  } else if (program.is_subcommand_used(helpCommand)) {
    showHelp(program);
  } else {
    errorMsg("Unknown option\n");
    showHelp(program);
  }

  return EXIT_SUCCESS;
}
