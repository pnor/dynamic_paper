#include "time_from_midnight.hpp"

#include "format.hpp"

namespace dynamic_paper {

// constexpr std::string TimeFromMidnight::toString() const {
//   return dynamic_paper::format("{} from Midnight ({})", this->seconds,
//                                dynamic_paper::format("{:%T}",
//                                this->seconds));
// }

// operator<< (printing)
std::ostream &operator<<(std::ostream &osStream,
                         const TimeFromMidnight &value) {
  osStream << value.seconds;
  return osStream;
}

} // namespace dynamic_paper
