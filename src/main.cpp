#include <format>
#include <iostream>

#include <argparse/argparse.hpp>
#include <spdlog/spdlog.h>
#include <tl/expected.hpp>
#include <yaml-cpp/yaml.h>

#include "background_set.hpp"
#include "cmdline_helper.hpp"
#include "config.hpp"
#include "defaults.hpp"
#include "file_util.hpp"
#include "logger.hpp"
#include "time_util.hpp"

using namespace dynamic_paper;

// TODO cache management (delete all and for one set + show location)
// TODO clang tidy
// TODO resolve ties in times not as assert failure (as can happen by accident
// with changing sunrise/sunset)
// TODO try and stop using less shell commands? (exec family maybe)
// - use wallutils directly as a go package
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
    std::cout << "Unable to show background set with name " << name
              << std::endl;
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

void showHelp(const argparse::ArgumentParser &program) {
  std::cout << program.help().str();
}

} // namespace

// ===== Main ===============

auto main(int argc, char *argv[]) -> int {
  setLoggingToStderr();

  argparse::ArgumentParser program("dynamicpaper");
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

  // TODO
  // argparse::ArgumentParser cacheCommand("cache");
  // cacheCommand.add_description("Manage cache for interpolated images");
  // cacheCommand.add_argument("name").help("Name of wallpaper set to show cache
  // information");

  program.add_subparser(showCommand);
  program.add_subparser(randomCommand);
  program.add_subparser(listCommand);
  program.add_subparser(helpCommand);

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error &err) {
    errorMsg("Error parsing command line options:\n{}", err.what());
    return EXIT_FAILURE;
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
  } else if (program.is_subcommand_used(helpCommand)) {
    showHelp(program);
  } else {
    errorMsg("Unknown option\n");
    showHelp(program);
  }

  return EXIT_SUCCESS;
}
