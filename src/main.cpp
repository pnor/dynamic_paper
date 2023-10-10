#include <iostream>

#include "./lib/argparse.hpp"
#include "config.hpp"

static void handleShowCommand(argparse::ArgumentParser &showCommand) {
  std::string wallpaperName = showCommand.get("name");
  std::cout << wallpaperName << std::endl;

  dynamic_paper::Config cfg =
      dynamic_paper::loadConfigFromFile("./files/test.yaml");
}

static void handleRandomCommand() { std::cout << "random" << std::endl; }

static void handleListCommand() { std::cout << "list " << std::endl; }

auto main(int argc, char *argv[]) -> int {
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
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  if (program.is_subcommand_used(showCommand)) {
    handleShowCommand(showCommand);
  } else if (program.is_subcommand_used(listCommand)) {
    handleListCommand();
  } else if (program.is_subcommand_used(randomCommand)) {
    handleRandomCommand();
  }

  return 0;
}
