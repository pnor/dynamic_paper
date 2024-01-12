#include <expected>
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

// ===== Helper ================
constexpr std::string_view ANSI_COLOR_RED = "\x1b[31m";
constexpr std::string_view ANSI_COLOR_GREEN = "\x1b[32m";
constexpr std::string_view ANSI_COLOR_YELLOW = "\x1b[33m";
constexpr std::string_view ANSI_COLOR_BLUE = "\x1b[34m";
constexpr std::string_view ANSI_COLOR_MAGENTA = "\x1b[35m";
constexpr std::string_view ANSI_COLOR_CYAN = "\x1b[36m";
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
  LogLevel logLevel = loadLoggingLevelFromYAML(config);
  setupLogging(logLevel);
}

// ===== Command Line Arguements ===============
YAML::Node loadConfigFileIntoYAML(const std::filesystem::path &file) {
  if (!std::filesystem::exists(file)) {
    errorMsg("Tried to load config file `{}` but it does not exist!",
             file.string());
    exit(EXIT_FAILURE);
  }

  try {
    return YAML::LoadFile(file);
  } catch (const YAML::BadFile &e) {
    errorMsg("Could not parse config file `{}`", file.string());
    exit(EXIT_FAILURE);
  }
}

Config createConfigFromYAML(const YAML::Node configYaml) {
  std::expected<Config, ConfigError> expConfig = loadConfigFromYAML(configYaml);

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

// ===== Command Line Arguements ===============

static void handleShowCommand(argparse::ArgumentParser &showCommand,
                              const Config &config) {
  const std::string &name = showCommand.get("name");

  std::optional<BackgroundSet> optBackgroundSet =
      getBackgroundSetWithNameFromFile(name, config);

  if (!optBackgroundSet.has_value()) {
    std::cout << "Unable to show background set with name " << name
              << std::endl;
    return;
  }

  optBackgroundSet->show(config);
}

static void handleRandomCommand(const Config &config) {
  std::optional<BackgroundSet> optBackgroundSet =
      getRandomBackgroundSet(config);

  if (!optBackgroundSet.has_value()) {
    std::cout << "Unable to parse any background set from the config file at : "
              << config.backgroundSetConfigFile.string() << std::endl;
    return;
  }

  optBackgroundSet->show(config);
}

static void handleListCommand(const Config &config) {
  std::vector<BackgroundSet> backgroundSets = getBackgroundSetsFromFile(config);
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

  program.add_argument("--config")
      .help("Show optional config")
      .default_value("~/.config/dynamic_paper/config.yaml");

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
  // create config location if it doesn't exist (if looking for default)
  // take a config from cmd line args
  YAML::Node configYaml = loadConfigFileIntoYAML("./files/test.yaml");

  setupLogging(configYaml);

  Config config = createConfigFromYAML(configYaml);
  // TODO create cache dir if not existing  already
  // TODO create default dirs like .local/share and .cache/dynamic_paper if they
  // don't exist
  // TODO create default config if it doesn't exist already

  bool result = parseArguements(argc, argv, config);

  return result ? 0 : 1;
}
