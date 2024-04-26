#include "background_test_setter.hpp"

#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "src/background_set_enums.hpp"
#include "src/logger.hpp"

namespace dynamic_paper_test {

namespace {

constexpr std::string setEventString(const SetEvent &event) {
  switch (event.mode) {
  case dynamic_paper::BackgroundSetMode::Center: {
    return "[mode = Center | " + event.imagePath.string() + " ]";
  };
  case dynamic_paper::BackgroundSetMode::Fill: {
    return "[mode = Fill   | " + event.imagePath.string() + " ]";
  };
  case dynamic_paper::BackgroundSetMode::Tile: {
    return "[mode = Tile   | " + event.imagePath.string() + " ]";
  };
  case dynamic_paper::BackgroundSetMode::Scale: {
    return "[mode = Scale   | " + event.imagePath.string() + " ]";
  };
  }
  dynamic_paper::logAssert(false,
                           "Unreachable section reached in `setEventString`");
  return "[?]";
}

} // namespace

// ===== Header =====

std::ostream &operator<<(std::ostream &osStream, const SetEvent &value) {
  osStream << setEventString(value);
  return osStream;
}

[[nodiscard]] const std::vector<SetEvent> &
TestBackgroundSetterHistory::getHistory() const {
  return this->setHistory;
}

void TestBackgroundSetterHistory::addEvent(SetEvent event) {
  this->setHistory.push_back(std::move(event));
}

} // namespace dynamic_paper_test
