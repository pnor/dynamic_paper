#pragma once

#include <filesystem>
#include <ostream>

#include "src/background_set_enums.hpp"

namespace dynamic_paper_test {

/** An example of an action to set the background */
struct SetEvent {
  std::filesystem::path imagePath;
  dynamic_paper::BackgroundSetMode mode;

  friend bool operator==(const SetEvent &lhs, const SetEvent &rhs) = default;
};

std::ostream &operator<<(std::ostream &osStream, const SetEvent &value);

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
