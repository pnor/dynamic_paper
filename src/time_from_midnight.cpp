#include "time_from_midnight.hpp"

#include <format>

#include "logger.hpp"
#include "time_util.hpp"

namespace dynamic_paper {

TimeFromMidnight TimeFromMidnight::forTime(const std::string_view timeString) {
  const std::optional<TimeFromMidnight> optTime =
      convertRawTimeStringToTimeOffset(timeString);
  logAssert(optTime.has_value(),
            "parsed time string should have value, but did not");
  return optTime.value();
}

// operator<< (printing)
std::ostream &operator<<(std::ostream &osStream,
                         const TimeFromMidnight &value) {
  osStream << value.seconds;
  return osStream;
}

} // namespace dynamic_paper
