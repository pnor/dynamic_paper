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

using namespace dynamic_paper;

namespace {

// ===== Helper ================
constexpr std::string_view ANSI_COLOR_RED = "\x1b[31m";
// constexpr std::string_view ANSI_COLOR_GREEN = "\x1b[32m";
// constexpr std::string_view ANSI_COLOR_YELLOW = "\x1b[33m";
// constexpr std::string_view ANSI_COLOR_BLUE = "\x1b[34m";
// constexpr std::string_view ANSI_COLOR_MAGENTA = "\x1b[35m";
// constexpr std::string_view ANSI_COLOR_CYAN = "\x1b[36m";
constexpr std::string_view ANSI_COLOR_RESET = "\x1b[0m";

template <typename... Ts>
void errorMsg(const std::format_string<Ts...> msg, Ts &&...args) {
  const std::string formattedMessage =
      std::format(msg, std::forward<Ts>(args)...);
  std::cout << std::format("{}{}{}", ANSI_COLOR_RED, formattedMessage,
                           ANSI_COLOR_RESET)
            << std::endl;
}

// ===== Logging ===============

void setupLogging(const YAML::Node &config) {
  const LogLevel logLevel = loadLoggingLevelFromYAML(config);
  setupLogging(logLevel);
}

// ===== Config ===============

YAML::Node loadConfigFileIntoYAML(const std::filesystem::path &file) {
  const bool fileCreationResult =
      createFileIfDoesntExist(file, DEFAULT_CONFIG_FILE);

  if (!fileCreationResult) {
    errorMsg("Unable to create a default config!");
    exit(EXIT_FAILURE);
  }

  try {
    return YAML::LoadFile(file);
  } catch (const YAML::BadFile &e) {
    errorMsg("Could not parse config file `{}`", file.string());
    exit(EXIT_FAILURE);
  }
}

Config createConfigFromYAML(const YAML::Node &configYaml) {
  tl::expected<Config, ConfigError> expConfig = loadConfigFromYAML(configYaml);

  if (!expConfig.has_value()) {
    switch (expConfig.error()) {
    case ConfigError::MethodParsingError: {
      errorMsg("Unable to parse general config; invalid method");
      break;
    }
    }
    exit(1);
  }

  return expConfig.value();
}

Config getConfigAndSetupLogging(const argparse::ArgumentParser &program) {
  const std::filesystem::path configFilePath =
      std::filesystem::path(program.get("--config"));

  const YAML::Node configYaml = loadConfigFileIntoYAML(configFilePath);

  setupLogging(configYaml);

  return createConfigFromYAML(configYaml);
}

// ===== h ===============

inline void showBackgroundSet(BackgroundSet &backgroundSet,
                              const Config &config) {
  std::optional<StaticBackgroundData> staticData =
      backgroundSet.getStaticBackgroundData();
  if (staticData.has_value()) {
    staticData->show(config);
  }

  std::optional<DynamicBackgroundData> dynamicData =
      backgroundSet.getDynamicBackgroundData();
  if (dynamicData.has_value()) {
    while (true) {
      const std::chrono::seconds sleepTime =
          dynamicData->updateBackground(config) + std::chrono::seconds(1);

      logDebug("Sleeping for {} seconds...", sleepTime);
      std::this_thread::sleep_for(sleepTime);
    }
  }
}

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

  showBackgroundSet(optBackgroundSet.value(), config);
}

void handleListCommand(const Config &config) {
  const std::vector<BackgroundSet> backgroundSets =
      getBackgroundSetsFromFile(config);
  std::cout << "Available Background sets are: "
            << "\n"
            << std::endl;
  for (const auto &backgroundSet : backgroundSets) {
    std::cout << backgroundSet.getName() << std::endl;
  }
}

void showHelp(const argparse::ArgumentParser &program) {
  std::cout << program.help().str();
}

} // namespace

// ===== Main ===============

auto main(int argc, char *argv[]) -> int {
  argparse::ArgumentParser program("dynamicpaper");
  program.add_argument("--config")
      .help("Show optional config")
      .default_value("~/.config/dynamic_paper/config.yaml")
      .implicit_value("");

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
    errorMsg("Error parsing command line options:\n{}", err.what());
    return EXIT_FAILURE;
  }

  // TODO create default dirs like .local/share and .cache/dynamic_paper if
  // they don't exist

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
