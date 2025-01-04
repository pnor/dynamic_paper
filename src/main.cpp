#include <iostream>

#include <argparse/argparse.hpp>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>
#include <tl/expected.hpp>
#include <yaml-cpp/yaml.h>

// Include this last
// Defines C macros that mess with the other libraries
// (yaml-cpp in particular by defining an "IsNaN" macro)
#include <Magick++.h>

#include "background_set.hpp"
#include "cmdline_helper.hpp"
#include "config.hpp"
#include "defaults.hpp"
#include "logger.hpp"

using namespace dynamic_paper;

// TODO replace yaml-cpp since it has some weirdness
// - Magick++.h messes with it if included before
// - deref with [] on a key that doesnt exist returns a valid "undefined" Node,
//   but also throws so you can't really catch it and run an undefined Node
//   branch
// TODO gpu composit image using opencv ? optional depends ??
// TODO choose any 1 image selection, in any set
// TODO rehaul general config for cmdline options, then file then defualts? (!
// fixes a lot)
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
              << config.backgroundSetConfigFile.string() << '\n';
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
  try {
    std::cout << program.help().str();
  } catch (const std::exception &e) {
    logError("Encountered exception when trying to show help: {}", e.what());
  }
}

} // namespace

// ===== Main ===============

auto main(int argc, char *argv[]) -> int {
  Magick::InitializeMagick(*argv);

  argparse::ArgumentParser program("dynamic_paper");
  program.add_argument(CONFIG_FLAG_NAME)
      .default_value<std::string>(std::string(DEFAULT_CONFIG_FILE_NAME))
      .required()
      .help("Which config file to use for settings");
  program.add_argument(LOG_TO_STDOUT_FLAG_NAME)
      .flag()
      .help("Whether to log to stdout instead of a logfile");

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
    const Config config = getConfigAndSetupLogging(program, true);
    handleShowCommand(showCommand, config);
  } else if (program.is_subcommand_used(listCommand)) {
    const Config config = getConfigAndSetupLogging(program, false);
    handleListCommand(config);
  } else if (program.is_subcommand_used(randomCommand)) {
    const Config config = getConfigAndSetupLogging(program, true);
    handleRandomCommand(config);
  } else if (program.is_subcommand_used(cacheCommand)) {
    const Config config = getConfigAndSetupLogging(program, false);
    handleCacheCommand(config, cacheCommand, cacheInfoCommand);
  } else if (program.is_subcommand_used(helpCommand)) {
    showHelp(program);
  } else {
    errorMsg("Unknown option\n");
    showHelp(program);
  }

  return EXIT_SUCCESS;
}
