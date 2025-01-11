#include "time_from_midnight.hpp"

#include "format.hpp"

namespace dynamic_paper {

// operator<< (printing)
std::ostream &operator<<(std::ostream &osStream,
                         const TimeFromMidnight &value) {
  osStream << dynamic_paper::format(
      "{} from Midnight ({})", value.seconds,
      dynamic_paper::format("{:%T}", value.seconds));
  return osStream;
}

} // namespace dynamic_paper
