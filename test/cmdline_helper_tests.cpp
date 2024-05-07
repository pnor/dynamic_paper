/**
 *   Test Functions used in the cmdline main loop
 */

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <gtest/gtest.h>

#include "helper.hpp"
#include "src/background_set.hpp"
#include "src/cmdline_helper.hpp"
#include "src/config.hpp"
#include "src/dynamic_background_set.hpp"
#include "src/static_background_set.hpp"

using namespace dynamic_paper;

namespace {

constexpr std::string_view TEST_BACKGROUND_SET_FILE =
    "./files/test_background_sets.yaml";

std::string getDataFromSet(BackgroundSet &backgroundSet) {
  const std::optional<StaticBackgroundData> staticData =
      backgroundSet.getStaticBackgroundData();

  if (staticData.has_value()) {
    return staticData->imageDirectory / *(staticData->imageNames.begin());
  }

  const DynamicBackgroundData dynamicData =
      backgroundSet.getDynamicBackgroundData().value();

  return dynamicData.imageDirectory / *(dynamicData.imageNames.begin());
}

} // namespace

// ===== Tests ===============

/**
 * Tests an issue with getting random background sets from a file is fixed.
 * Grabs the first image each background set would produce and compares it. If
 * the image from subsequent calls to `getRandomBackgroundSet` is different,
 * passes the test. If for `NUM_REPITITIONS`, the image remains the same, then
 * fails (as that ould imply we are randomly choosing the same background set
 * over and over).
 *
 * (note that this test has a chance to fail even if random works due to the
 * nature of randomness,. This chance gets smaller the greater NUM_REPITITIONS
 * is)
 */
TEST(ChooseRandomBackgroundSet, CmdlineMainLoop) {
  EXPECT_TRUE(
      std::filesystem::exists(std::filesystem::path(TEST_BACKGROUND_SET_FILE)));

  const Config config =
      getConfig(std::filesystem::path(TEST_BACKGROUND_SET_FILE));

  std::optional<BackgroundSet> optBackgroundSet = std::nullopt;

  constexpr int NUM_REPITITIONS = 500;
  std::optional<std::string> lastChoice = std::nullopt;

  for (int i = 0; i < NUM_REPITITIONS; i++) {
    optBackgroundSet = getRandomBackgroundSet(config);
    ASSERT_TRUE(optBackgroundSet.has_value());

    std::string identifyingInfoFromBackgroundSet =
        getDataFromSet(optBackgroundSet.value());

    if (lastChoice.has_value() &&
        lastChoice.value() != identifyingInfoFromBackgroundSet) {
      return;
    }

    lastChoice = std::move(identifyingInfoFromBackgroundSet);
  }

  FAIL();
}
