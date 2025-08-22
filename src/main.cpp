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
#include "background_set_enums.hpp"
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

constexpr std::string_view ANSI_BOLD = "\x1b[0m";
constexpr std::string_view ANSI_COLOR_CYAN = "\x1b[36m";
constexpr std::string_view ANSI_COLOR_RED = "\x1b[31m";
constexpr std::string_view ANSI_COLOR_MAGENTA = "\x1b[35m";
constexpr std::string_view ANSI_COLOR_RESET = "\x1b[0m";

void showRandomBackgroundSet(
    const Config &config,
    const std::optional<BackgroundSetMode> optMode = std::nullopt) {
  std::optional<BackgroundSet> optBackgroundSet =
      getRandomBackgroundSet(config);

  if (!optBackgroundSet.has_value()) {
    std::cout << "Unable to parse any background set from the config file at : "
              << config.backgroundSetConfigFile.string() << '\n';
    return;
  }

  logDebug("Showing background set: {}", optBackgroundSet->getName());

  showBackgroundSet(optBackgroundSet.value(), config, optMode);
}

void showRandomImageFromAllBackgroundSets(
    const Config &config, const std::optional<BackgroundSetMode> optMode) {
  const std::optional<std::pair<std::filesystem::path, BackgroundSetMode>>
      imageAndModeOpt = getRandomImageAndModeFromAllBackgroundSets(config);

  if (!imageAndModeOpt) {
    std::cout << "Unable to parse any images from the config file at : "
              << config.backgroundSetConfigFile.string() << "\n";
    return;
  }

  const auto &[image, mode] = imageAndModeOpt.value();

  logDebug("Showing image: {} with mode {}", image.string(),
           backgroundSetModeString(mode));

  if (shouldUseScriptToSetBackground(config)) {
    useScriptToSetBackground(config, image, optMode.value_or(mode));
  } else {
    setBackgroundToImage(image, optMode.value_or(mode));
  }

  std::cout << "Set background to " << ANSI_COLOR_CYAN << image.string()
            << ANSI_COLOR_RESET << "\n";

  if (config.hookScript) {
    runHookScript(config.hookScript.value(), image);
  }
}

// ===== Command Line Arguements ===============

void handleShowCommand(argparse::ArgumentParser &showCommand,
                       const Config &config) {
  const std::string &name = showCommand.get("name");
  const std::string modeString = showCommand.present("--mode").value_or("");
  const std::optional<BackgroundSetMode> mode =
      stringToBackgroundSetMode(modeString);

  if (std::filesystem::is_regular_file(name)) {
    logDebug("Showing image path {}", name);

    if (shouldUseScriptToSetBackground(config)) {
      useScriptToSetBackground(config, name,
                               mode.value_or(BackgroundSetMode::Scale));
    } else {
      setBackgroundToImage(name, mode.value_or(BackgroundSetMode::Scale));
    }
  } else {
    std::optional<BackgroundSet> optBackgroundSet =
        getBackgroundSetWithNameFromFile(name, config);

    if (!optBackgroundSet.has_value()) {
      std::cout << "Unable to show background set with name " << name << '\n';
      return;
    }

    showBackgroundSet(optBackgroundSet.value(), config, mode);
  }
}

void handleRandomCommand(argparse::ArgumentParser &randomCommand,
                         const Config &config) {
  const std::string modeString = randomCommand.present("--mode").value_or("");
  const std::optional<BackgroundSetMode> optMode =
      stringToBackgroundSetMode(modeString);

  if (randomCommand["--image"] == true) {
    showRandomImageFromAllBackgroundSets(config, optMode);
  } else {
    showRandomBackgroundSet(config, optMode);
  }
}

void handleListCommand(argparse::ArgumentParser &listCommand,
                       const Config &config) {
  if (!std::filesystem::exists(config.backgroundSetConfigFile)) {
    errorMsg("No config file exists for background sets at path: {}",
             config.backgroundSetConfigFile.string());
    exit(EXIT_FAILURE);
  }

  const std::vector<BackgroundSet> backgroundSets =
      getBackgroundSetsFromFile(config);

  const std::vector<std::pair<std::string_view, BackgroundSetType>>
      namesAndTypes = getNamesAndTypes(backgroundSets);

  if (listCommand["--no-format"] == true || isBeingPiped()) {
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

void handleInfoCommand(argparse::ArgumentParser &showCommand,
                       const Config &config) {
  const std::string &name = showCommand.get("name");

  std::optional<BackgroundSet> optBackgroundSet =
      getBackgroundSetWithNameFromFile(name, config);

  if (!optBackgroundSet.has_value()) {
    std::cout << ANSI_COLOR_RED << "Background set with name " << name
              << " doesn't exist!\n"
              << ANSI_COLOR_RESET;
    return;
  }

  printBackgroundSetInfo(optBackgroundSet.value());
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
  showCommand.add_description("Show image or wallpaper set with name");
  showCommand.add_argument("name").help(
      "Image path or name of wallpaper set to show");
  showCommand.add_argument("--mode", "-m")
      .help("Center, Fill, Tile, or Scale (Background Set config specified or "
            "Scale by default)");

  argparse::ArgumentParser randomCommand("random");
  randomCommand.add_description("Show a random wallpaper set");
  randomCommand.add_argument("--image")
      .help("Choose one wallpaper to show out of all available sets instead of "
            "choosing one background set at random")
      .flag();
  randomCommand.add_argument("--mode", "-m")
      .help("Center, Fill, Tile, or Scale (Background Set config specified or "
            "Scale by default)");

  argparse::ArgumentParser listCommand("list");
  listCommand.add_description("List all wallpaper set options");
  listCommand.add_argument("--no-format")
      .help("Print available backgrounds without formatting")
      .flag();

  argparse::ArgumentParser infoCommand("info");
  infoCommand.add_description("Show info for wallpaper set with name");
  infoCommand.add_argument("name").help("Name of wallpaper set to describe");

  argparse::ArgumentParser helpCommand("help");
  helpCommand.add_description("Show help");

  argparse::ArgumentParser cacheCommand("cache");
  cacheCommand.add_description("Manage cache for interpolated images");
  argparse::ArgumentParser cacheInfoCommand("info");
  cacheInfoCommand.add_description(
      "Show information about cached interpolated images");
  cacheCommand.add_subparser(cacheInfoCommand);

  program.add_subparser(showCommand);
  program.add_subparser(randomCommand);
  program.add_subparser(listCommand);
  program.add_subparser(infoCommand);
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
    handleListCommand(listCommand, config);
  } else if (program.is_subcommand_used(infoCommand)) {
    const Config config = getConfigAndSetupLogging(program, true);
    handleInfoCommand(infoCommand, config);
  } else if (program.is_subcommand_used(randomCommand)) {
    const Config config = getConfigAndSetupLogging(program, true);
    handleRandomCommand(randomCommand, config);
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
