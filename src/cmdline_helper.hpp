#pragma once

#include <argparse/argparse.hpp>

#include "background_set.hpp"
#include "config.hpp"

namespace dynamic_paper {

/** Helper function for `main` that handle the user facing functionality of the
 * program */

// ===== Logging ===============

template <typename... Ts>
void errorMsg(const std::format_string<Ts...> msg, Ts &&...args) {
  constexpr std::string_view ANSI_COLOR_RED = "\x1b[31m";
  constexpr std::string_view ANSI_COLOR_RESET = "\x1b[0m";

  const std::string formattedMessage =
      std::format(msg, std::forward<Ts>(args)...);
  std::cout << std::format("{}{}{}", ANSI_COLOR_RED, formattedMessage,
                           ANSI_COLOR_RESET)
            << std::endl;
}

// ===== Parsing ===============

/**
 * Sets up logging and parses the Config file.
 * Exits if there is an error parsing a file or creating files.
 *
 * Creates a sample config file if none exists and the config is set to the
 * default.
 * */
Config getConfigAndSetupLogging(const argparse::ArgumentParser &program);

// ===== Cache ===============

/** Shows information about cached files */
void showCacheInfo(const Config &config);

// ===== Command Line Args ===============

/** Returns `true` if the output from this process is being piped into another
 * pipe or file */
bool isBeingPiped();

// ===== Background Set ===============

/**
 * Parses `BackgroundSet`s from the config file.
 *
 * Exits the program if encounters an error parsing `backgroundSetFile`
 * Does not add `BackgroundSet`s that errored out when parsing
 */
std::vector<BackgroundSet> getBackgroundSetsFromFile(const Config &config);

/**
 * Gets a list of all the names of every background set along with its type,
 * sorted in alphabetical order.
 */
std::vector<std::pair<std::string_view, BackgroundSetType>>
getNamesAndTypes(const std::vector<BackgroundSet> &backgroundSets);

/**
 * Returns the `BackgroundSet` called `name` from the config file, returning
 * `nullopt` if unable to find one, or unable to succesfully parse it
 */
std::optional<BackgroundSet>
getBackgroundSetWithNameFromFile(std::string_view name, const Config &config);

/**
 * Returns a random `BackgroundSet` from the config file.
 *
 * If unable to parse any background sets, will return nullopt
 */
std::optional<BackgroundSet> getRandomBackgroundSet(const Config &config);

/** Shows `backgroundSet` using user's `config`. */
void showBackgroundSet(BackgroundSet &backgroundSet, const Config &config);

} // namespace dynamic_paper
