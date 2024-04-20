#include "time_from_midnight.hpp"

namespace dynamic_paper {

// operator<< (printing)
std::ostream &operator<<(std::ostream &osStream,
                         const TimeFromMidnight &value) {
  osStream << value.seconds;
  return osStream;
}

} // namespace dynamic_paper
