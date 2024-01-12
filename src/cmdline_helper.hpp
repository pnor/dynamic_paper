#pragma once

#include "background_set.hpp"
#include "config.hpp"

namespace dynamic_paper {

/** Helper function for `main` that handle the user facing functionality of the
 * program */

// ===== Command Line Args ===============
// ===== Background Set ===============

/**
 * Parses `BackgroundSet`s from the config file.
 *
 * Exits the program if encounters an error parsing `backgroundSetFile`
 * Does not add `BackgroundSet`s that errored out when parsing
 */
std::vector<BackgroundSet> getBackgroundSetsFromFile(const Config &config);

/**
 * Returns the `BackgroundSet` called `name` from the config file, returning
 * `nullopt` if unable to find one, or unable to succesfully parse it
 */
std::optional<BackgroundSet>
getBackgroundSetWithNameFromFile(const std::string_view name,
                                 const Config &config);

/**
 * Returns a random `BackgroundSet` from the config file.
 *
 * If unable to parse any background sets, will return nullopt
 */
std::optional<BackgroundSet> getRandomBackgroundSet(const Config &config);

} // namespace dynamic_paper
