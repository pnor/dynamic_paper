#pragma once

#include <filesystem>

#include <tl/expected.hpp>

#include "src/background_set_enums.hpp"

namespace dynamic_paper_test {

using namespace dynamic_paper;

/** An example of an action to set the background */
struct SetEvent {
  std::filesystem::path imagePath;
  BackgroundSetMode mode;

  SetEvent(std::filesystem::path imagePath, BackgroundSetMode mode);

  friend bool operator==(const SetEvent &lhs, const SetEvent &rhs) = default;
};

/** Test class that stores a global history of backgrounds that were set
 * */
class TestBackgroundSetterHistory {
public:
  /**
   * Return vector of all calls to change the background
   */
  [[nodiscard]] const std::vector<SetEvent> &getHistory() const;

  /**
   * Adds a new call to change the background
   */
  void addEvent(SetEvent event);

private:
  std::vector<SetEvent> setHistory;
};
} // namespace dynamic_paper_test
