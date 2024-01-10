#include <format>
#include <iostream>

#include <argparse/argparse.hpp>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include "background_set.hpp"
#include "cmdline_helper.hpp"
#include "config.hpp"
#include "logger.hpp"

using namespace dynamic_paper;

// TODO replace
const std::filesystem::path
    BACKGROUND_SET_CONFIG("./files/background_sets.yaml");

void setupLogging(const YAML::Node &config) {
  LogLevel logLevel = loadLoggingLevelFromYAML(config);
  setupLogging(logLevel);
}

static void handleShowCommand(argparse::ArgumentParser &showCommand,
                              const Config &config) {
  const std::string &name = showCommand.get("name");

  std::optional<BackgroundSet> optBackgroundSet =
      getBackgroundSetWithNameFromFile(name, BACKGROUND_SET_CONFIG, config);

  if (!optBackgroundSet.has_value()) {
    std::cout << "Unable to show background set with name " << name
              << std::endl;
    return;
  }

  optBackgroundSet->show(config);
}

static void handleRandomCommand(const Config &config) {
  std::optional<BackgroundSet> optBackgroundSet =
      getRandomBackgroundSet(BACKGROUND_SET_CONFIG, config);

  if (!optBackgroundSet.has_value()) {
    std::cout << "Unable to parse any background set from the config file at : "
              << BACKGROUND_SET_CONFIG.string() << std::endl;
    return;
  }

  optBackgroundSet->show(config);
}

static void handleListCommand(const Config &config) {
  std::vector<BackgroundSet> backgroundSets =
      getBackgroundSetsFromFile(BACKGROUND_SET_CONFIG, config);
  std::cout << "Available Background sets are: "
            << "\n"
            << std::endl;
  for (const auto &b : backgroundSets) {
    std::cout << b.name << std::endl;
  }
}

static void showHelp(const argparse::ArgumentParser &program) {
  std::cout << program.help().str();
}

static bool parseArguements(const int argc, char *argv[],
                            const Config &config) {
  argparse::ArgumentParser program("dynamic paper");

  argparse::ArgumentParser showCommand("show");
  showCommand.add_description("Show wallpaper set with name");
  showCommand.add_argument("name").help("Name of wallpaper set to show");

  argparse::ArgumentParser randomCommand("random");
  randomCommand.add_description("Show a random wallpaper set");

  argparse::ArgumentParser listCommand("list");
  listCommand.add_description("List all wallpaper set options");

  argparse::ArgumentParser helpCommand("help");
  listCommand.add_description("Show help");

  program.add_subparser(showCommand);
  program.add_subparser(randomCommand);
  program.add_subparser(listCommand);
  program.add_subparser(helpCommand);

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error &err) {
    std::cout << "Error parsing command line options:" << std::endl;
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return false;
  }

  if (program.is_subcommand_used(showCommand)) {
    handleShowCommand(showCommand, config);
  } else if (program.is_subcommand_used(listCommand)) {
    handleListCommand(config);
  } else if (program.is_subcommand_used(randomCommand)) {
    handleRandomCommand(config);
  } else if (program.is_subcommand_used(helpCommand)) {
    showHelp(program);
  } else {
    showHelp(program);
  }

  return true;
}

// ===== Main ===============

auto main(int argc, char *argv[]) -> int {
  YAML::Node configYaml = YAML::LoadFile("./files/test.yaml");

  setupLogging(configYaml);
  Config config = loadConfigFromYAML(configYaml).value();
  // TODO create cache dir if not existing  already

  bool result = parseArguements(argc, argv, config);

  return result ? 0 : 1;
}
