#include "background_test_setter.hpp"

namespace dynamic_paper_test {

using namespace dynamic_paper;

SetEvent::SetEvent(std::filesystem::path imagePath,
                   const BackgroundSetMode mode)
    : imagePath(std::move(imagePath)), mode(mode) {}

[[nodiscard]] const std::vector<SetEvent> &
TestBackgroundSetterHistory::getHistory() const {
  return this->setHistory;
}

void TestBackgroundSetterHistory::addEvent(SetEvent event) {
  this->setHistory.push_back(std::move(event));
}

} // namespace dynamic_paper_test
