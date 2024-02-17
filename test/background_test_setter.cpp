#include "background_test_setter.hpp"

namespace dynamic_paper_test {

using namespace dynamic_paper;

namespace {

constexpr std::string setEventString(const SetEvent &event) {
  switch (event.mode) {
  case dynamic_paper::BackgroundSetMode::Center: {
    return "[mode = Center | " + event.imagePath.string() + " ]";
  };
  case dynamic_paper::BackgroundSetMode::Fill: {
    return "[mode = Fill   | " + event.imagePath.string() + " ]";
  };
  }
  logAssert(false, "Unreachable");
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
