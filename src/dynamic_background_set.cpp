#include "dynamic_background_set.hpp"
#include "math_util.hpp"

#include <random>

namespace dynamic_paper {

namespace _helper {

// ===== Helper =====
namespace {

constexpr std::chrono::hours TWENTY_FOUR_HOURS(24);

/**
 * Class to be used by `std::shuffle` that uses `std::rand`.
 * Mainly for the ability to control the random order by calling `std::srand`
 *
 * Purposely uses `std::rand` in this case as the integrity of randomness for
 * choosing order of backgrounds is not that important, and is easily manipulatd
 * with `std::srand`.
 */
class RandUniformRandomBitGenerator {
public:
  using result_type = unsigned int;

  static constexpr result_type min() { return 0; }
  static constexpr result_type max() { return RAND_MAX; }
  static result_type g() { return std::rand(); }

  result_type operator()() { return g(); }
};

using namespace _helper;

/** Returns a time that is equivalent to `time` - `duration`, but clamped so
 * that it is always >= `after`. Accounts for the looping property of the time
 * */
std::pair<TimeFromMidnight, std::chrono::seconds>
nonOverlappingTimeAndDuration(TimeFromMidnight time,
                              const std::chrono::seconds duration,
                              TimeFromMidnight after) {
  const std::chrono::seconds timeSeconds = time;
  const std::chrono::seconds afterSeconds =
      after > time ? -(TWENTY_FOUR_HOURS - std::chrono::seconds(after))
                   : std::chrono::seconds(after);

  const std::chrono::seconds nonOverlappingSeconds =
      std::max(timeSeconds - duration, afterSeconds);
  return {TimeFromMidnight(nonOverlappingSeconds),
          time - nonOverlappingSeconds};
}

EventList createEventListFromTimesAndNames(
    const DynamicBackgroundData *dynamicData,
    const std::vector<std::pair<TimeFromMidnight, std::string>>
        &timesAndNames) {
  const std::optional<TransitionInfo> &transition = dynamicData->transition;

  EventList eventList;

  logAssert(!timesAndNames.empty(), "Times and names cannot be empty");

  // Single event case
  if (timesAndNames.size() == 1) {
    eventList.emplace_back(
        timesAndNames.begin()->first,
        SetBackgroundEvent{.imagePath = dynamicData->dataDirectory /
                                        timesAndNames.begin()->second});
    return eventList;
  }

  for (EventList::size_type i = 0; i < timesAndNames.size(); i++) {
    // transition event
    if (dynamicData->transition.has_value()) {
      const std::pair<TimeFromMidnight, std::string> &beforeTimeName =
          timesAndNames.at(mod(static_cast<int>(i) - 1,
                               static_cast<int>(timesAndNames.size())));
      const std::pair<TimeFromMidnight, std::string> &afterTimeName =
          timesAndNames.at(i);

      const auto [transitionTime, actualDuration] =
          nonOverlappingTimeAndDuration(
              afterTimeName.first, transition->duration,
              beforeTimeName.first + std::chrono::seconds(1));

      const LerpBackgroundEvent lerpEvent = {
          .commonImageDirectory = dynamicData->dataDirectory,
          .startImageName = beforeTimeName.second,
          .endImageName = afterTimeName.second,
          .transition = TransitionInfo(actualDuration, transition->steps)};

      eventList.emplace_back(transitionTime, lerpEvent);
    }
    // set background event
    eventList.emplace_back(
        timesAndNames[i].first,
        SetBackgroundEvent{.imagePath = dynamicData->dataDirectory /
                                        timesAndNames[i].second});
  }

  std::ranges::sort(eventList, {}, &std::pair<TimeFromMidnight, Event>::first);

  return eventList;
}

/**
 * Shuffles a vector using `std::rand()` as the source of randomness
 */
template <typename T> void shuffleVector(std::vector<T> &vec) {
  std::shuffle(vec.begin(), vec.end(), RandUniformRandomBitGenerator());
}

/** Return out readable string describing what an event does */
std::string getEventImageName(const Event &event) {
  return std::visit(overloaded{[](const SetBackgroundEvent &event) {
                                 return event.imagePath.filename().string();
                               },
                               [](const LerpBackgroundEvent &event) {
                                 return event.startImageName + " -> " +
                                        event.endImageName;
                               }},
                    event);
}

} // namespace

// ===== Header ===============

void describeError(const BackgroundError error) {
  switch (error) {
  case BackgroundError::CommandError: {
    logError("Error occured when trying to interpolate the background: eror "
             "when running a command");
    break;
  }
  case BackgroundError::CompositeImageError: {
    logError("Error occured when trying to interpolate the background: unable "
             "to create composite image");
    break;
  }
  case BackgroundError::SetBackgroundError: {
    logError(
        "Error occured when trying to interpolate the background: unable to "
        "set the background");
    break;
  }
  case BackgroundError::NoCacheDir: {
    logError("Error occured when trying to interpolate the background: unable "
             "to access or create a cache directory");
    break;
  }
  }
}

unsigned int chooseRandomSeed() {
  std::random_device random_device;
  std::mt19937 generator(random_device());

  using limits = std::numeric_limits<unsigned int>;

  std::uniform_int_distribution<unsigned int> uniformDist(limits::min(),
                                                          limits::max());
  return uniformDist(generator);
}

/**
 * Returns true if eventList is sorted in ascending order, with no 2 times being
 * the same
 */
bool eventListIsSortedByTime(const EventList &eventList) {
  for (size_t i = 0; i < eventList.size() - 1; i++) {
    if (!(eventList[i].first < eventList[i + 1].first)) {
      return false;
    }
  }
  return true;
}

/**
 * Returns the times and names in `dynamicData` sorted by time.
 * Preserves the pair relation between the names and times.
 * Example: if a time was at index 1 in times and a name was at index 1 in
 * names, they would appear in the same pair in the sorted output.
 **/
std::vector<std::pair<TimeFromMidnight, std::string>>
timesAndNamesSortedByTime(const DynamicBackgroundData *dynamicData) {
  std::vector<std::pair<TimeFromMidnight, std::string>> timesNames;
  const std::vector<TimeFromMidnight> &times = dynamicData->times;
  const std::vector<std::string> &imageNames = dynamicData->imageNames;

  for (size_t i = 0; i < std::min(times.size(), imageNames.size()); i++) {
    timesNames.emplace_back(times[i], imageNames[i]);
  }

  std::ranges::sort(timesNames, {},
                    &std::pair<TimeFromMidnight, std::string>::first);

  return timesNames;
}

/**
 * Returns the times and names in `dynamicData` sorted by time, but with the
 * image name chosen randomly.
 */
std::vector<std::pair<TimeFromMidnight, std::string>>
timesAndRandomNamesSortedByTime(const DynamicBackgroundData *dynamicData) {
  std::vector<std::pair<TimeFromMidnight, std::string>> timesAndNames;

  std::vector<std::string> names = dynamicData->imageNames;
  shuffleVector(names);

  for (size_t i = 0; i < dynamicData->times.size(); i++) {
    timesAndNames.emplace_back(dynamicData->times[i], names[i % names.size()]);
  }

  return timesAndNames;
}

EventList getEventList(const DynamicBackgroundData *dynamicData) {
  EventList eventList;

  switch (dynamicData->order) {
  case BackgroundSetOrder::Linear: {
    eventList = createEventListFromTimesAndNames(
        dynamicData, timesAndNamesSortedByTime(dynamicData));
    break;
  }
  case BackgroundSetOrder::Random: {
    eventList = createEventListFromTimesAndNames(
        dynamicData, timesAndRandomNamesSortedByTime(dynamicData));
    break;
  }
  }

  return eventList;
}

std::chrono::seconds getEventDuration(const Event &event) {
  return std::visit(overloaded{[](const SetBackgroundEvent & /*unused*/) {
                                 return std::chrono::seconds(0);
                               },
                               [](const LerpBackgroundEvent &event) {
                                 return event.transition.duration;
                               }},
                    event);
}

std::pair<TimeAndEvent, TimeFromMidnight>
getCurrentEventAndNextTime(const EventList &eventList,
                           const TimeFromMidnight time) {
  logAssert(!eventList.empty(), "Event list is empty");

  auto firstAfterTime =
      std::ranges::find_if(eventList, [time](const TimeAndEvent &timeAndEvent) {
        return timeAndEvent.first > time;
      });

  if (firstAfterTime == eventList.begin() ||
      firstAfterTime == eventList.end()) {
    const TimeAndEvent &current = eventList.back();
    const TimeFromMidnight next = eventList.front().first;

    return std::make_pair(current, next);
  }

  const TimeAndEvent current = *(firstAfterTime - 1);
  const TimeFromMidnight next = firstAfterTime->first;

  return std::make_pair(current, next);
}

std::chrono::seconds timeUntilNext(const TimeFromMidnight &now,
                                   const std::chrono::seconds eventDuration,
                                   const TimeFromMidnight &later) {
  std::chrono::seconds sleepTime;

  if (now == later) {
    sleepTime = TWENTY_FOUR_HOURS - eventDuration;
  } else if (now < later) {
    sleepTime = std::chrono::seconds(later - now) - eventDuration;
  } else { // Sleep past a day boundary / now >= later
    constexpr TimeFromMidnight secondBeforeMidnight =
        convertRawTimeStringToTimeOffsetUnchecked("23:59:59");
    constexpr TimeFromMidnight midnight =
        convertRawTimeStringToTimeOffsetUnchecked("00:00:00");

    sleepTime =
        std::chrono::seconds((secondBeforeMidnight - now) +
                             std::chrono::seconds(1) + (later - midnight)) -
        eventDuration;
  }

  return std::clamp(sleepTime, std::chrono::seconds(0),
                    std::chrono::seconds(TWENTY_FOUR_HOURS));
}

void logPrintEventList(const EventList &eventList) {
  logInfo("Entire event list:");
  for (const auto &event : eventList) {
    logInfo("{} : {}", event.first, getEventImageName(event.second));
  }
  logInfo("--------");
}

} // namespace _helper

DynamicBackgroundData::DynamicBackgroundData(
    std::filesystem::path dataDirectory, BackgroundSetMode mode,
    std::optional<TransitionInfo> transition, BackgroundSetOrder order,
    std::vector<std::string> imageNames, std::vector<TimeFromMidnight> times)
    : dataDirectory(std::move(dataDirectory)), mode(mode),
      transition(transition), order(order), imageNames(std::move(imageNames)),
      times(std::move(times)) {}
} // namespace dynamic_paper
