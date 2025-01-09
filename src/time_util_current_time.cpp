#include "time_util_current_time.hpp"

#include "format.hpp"
#include "logger.hpp"
#include "time_util.hpp"

#ifndef dynamic_paper_use_std_chrono_zoned_time
#include <iomanip>
#include <sstream>
#endif

namespace dynamic_paper {

namespace {

/** `formattedString` shoule be formatted HH:MM:SS */
TimeFromMidnight timeFromString(const std::string_view formattedString) {
  // HH:MM:SS
  constexpr size_t HOURS_MINUTES_SIZE = 8;

  logDebug("Current time unparsed is {}", formattedString);
  std::optional<TimeFromMidnight> optTime = convertTimeStringToTimeFromMidnight(
      formattedString.substr(0, HOURS_MINUTES_SIZE));

  logAssert(optTime.has_value(), "Unable to parse valid time from return "
                                 "result of current time as string");

  return optTime.value();
}

} // namespace

#ifdef dynamic_paper_use_std_chrono_zoned_time
TimeFromMidnight getCurrentTime() {
  constexpr size_t START_OF_LOCAL_TIME = 11;

  const std::chrono::zoned_time zonedTime{std::chrono::current_zone(),
                                          std::chrono::system_clock::now()};

  // TODO figure out way to format without std::format to respect that flag
  const std::string timeString =
      dynamic_paper::format("{}", zonedTime).substr(START_OF_LOCAL_TIME);

  return timeFromString(timeString);
}
#else
TimeFromMidnight getCurrentTime() {
  const auto now_c =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  std::stringstream stringStream;
  stringStream << std::put_time(std::localtime(&now_c), "%H:%M:%S") << "\n";

  const std::string timeString = stringStream.str();

  return timeFromString(timeString);
}
#endif
} // namespace dynamic_paper
