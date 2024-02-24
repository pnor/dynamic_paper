#include "dynamic_background_set.hpp"

#include <random>

namespace dynamic_paper {

namespace _helper {

// ===== Helper =====
namespace {

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

template <typename T> constexpr T mod(const T number, const T modulo) {
  return ((number % modulo) + modulo) % modulo;
}

/**
 * Return `time - secondsBefore`, looping the result up to the end of the day if
 * `time` is close to 00:00.
 *
 * For example:
 * If `secondsBefore` = 5 and `time` was the time_t representation of 02:30:00,
 * will return the time_t representation of 2:29:55.
 *
 * If `secondsBefore` = 5 and `time` was the time_t representation of 00:00:02,
 * will return the time_t representation of 23:59:57.
 */
constexpr time_t timeBefore(const time_t time,
                            std::chrono::seconds secondsBefore) {
  // convert time_t to a signed type for this modulo operation
  using TimeType = long;

  const TimeType seconds = secondsBefore.count();

  constexpr TimeType BEGINNING_OF_DAY =
      convertRawTimeStringToTimeOffset("00:00:00").value();
  constexpr TimeType END_OF_DAY =
      convertRawTimeStringToTimeOffset("23:59:59").value() + 1;
  constexpr TimeType DAY_LENGTH = END_OF_DAY - BEGINNING_OF_DAY;

  const TimeType result =
      (mod(((time - BEGINNING_OF_DAY) - seconds), DAY_LENGTH)) +
      BEGINNING_OF_DAY;

  return static_cast<time_t>(result);
}

EventList createEventListFromTimesAndNames(
    const DynamicBackgroundData *dynamicData,
    const std::vector<std::pair<time_t, std::string>> &timesAndNames) {
  const std::optional<TransitionInfo> &transition = dynamicData->transition;

  EventList eventList;

  // TODO clamp events to not make the lerping overlap and be out of order

  logAssert(!timesAndNames.empty(), "Times and names cannot be empty");

  if (transition.has_value() && timesAndNames.size() > 1) {
    const LerpBackgroundEvent lerpEvent = {
        .commonImageDirectory = dynamicData->dataDirectory,
        .startImageName = timesAndNames.back().second,
        .endImageName = timesAndNames.front().second,
        .transition = transition.value()};

    const time_t transitionTime =
        timeBefore(timesAndNames.front().first, transition->duration);

    eventList.emplace_back(transitionTime, lerpEvent);
  }
  eventList.emplace_back(
      timesAndNames[0].first,
      SetBackgroundEvent{.imagePath = dynamicData->dataDirectory /
                                      timesAndNames[0].second});

  // Add rest of events
  for (EventList::size_type i = 1; i < timesAndNames.size(); i++) {
    // transition event
    if (dynamicData->transition.has_value()) {
      const LerpBackgroundEvent lerpEvent = {
          .commonImageDirectory = dynamicData->dataDirectory,
          .startImageName = timesAndNames[i - 1].second,
          .endImageName = timesAndNames[i].second,
          .transition = transition.value()};

      const time_t transitionTime =
          timeBefore(timesAndNames[i].first, transition->duration);

      eventList.emplace_back(transitionTime, lerpEvent);
    }
    // set background event
    eventList.emplace_back(
        timesAndNames[i].first,
        SetBackgroundEvent{.imagePath = dynamicData->dataDirectory /
                                        timesAndNames[i].second});
  }

  // TODO should prob not sort like this; find way to build list in order
  std::ranges::sort(eventList, {}, &std::pair<time_t, Event>::first);

  return eventList;
}

/**
 * Shuffles a vector using `std::rand()` as the source of randomness
 */
template <typename T> void shuffleVector(std::vector<T> &vec) {
  std::shuffle(vec.begin(), vec.end(), RandUniformRandomBitGenerator());
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
std::vector<std::pair<time_t, std::string>>
timesAndNamesSortedByTime(const DynamicBackgroundData *dynamicData) {
  std::vector<std::pair<time_t, std::string>> timesNames;
  const std::vector<time_t> &times = dynamicData->times;
  const std::vector<std::string> &imageNames = dynamicData->imageNames;

  for (size_t i = 0; i < std::min(times.size(), imageNames.size()); i++) {
    timesNames.emplace_back(times[i], imageNames[i]);
  }

  std::ranges::sort(timesNames, {}, &std::pair<time_t, std::string>::first);

  return timesNames;
}

/**
 * Returns the times and names in `dynamicData` sorted by time, but with the
 * image name chosen randomly.
 */
std::vector<std::pair<time_t, std::string>>
timesAndRandomNamesSortedByTime(const DynamicBackgroundData *dynamicData) {
  std::vector<std::pair<time_t, std::string>> timesAndNames;

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

std::pair<TimeAndEvent, time_t>
getCurrentEventAndNextTime(const EventList &eventList, const time_t time) {
  logAssert(!eventList.empty(), "Event list is empty");

  auto firstAfterTime =
      std::ranges::find_if(eventList, [time](const TimeAndEvent &timeAndEvent) {
        return timeAndEvent.first > time;
      });

  if (firstAfterTime == eventList.begin() ||
      firstAfterTime == eventList.end()) {
    const TimeAndEvent &current = eventList.back();
    const time_t next = eventList.front().first;

    return std::make_pair(current, next);
  }

  const TimeAndEvent current = *(firstAfterTime - 1);
  const time_t next = firstAfterTime->first;

  return std::make_pair(current, next);
}

std::chrono::seconds timeUntilNext(const time_t &now,
                                   const std::chrono::seconds eventDuration,
                                   const time_t &later) {
  std::chrono::seconds sleepTime;

  constexpr std::chrono::hours TWENTY_FOUR_HOURS(24);

  if (now == later) {
    sleepTime = TWENTY_FOUR_HOURS - eventDuration;
  } else if (now < later) {
    sleepTime = std::chrono::seconds(later - now) - eventDuration;
  } else { // Sleep past a day boundary / now >= later
    constexpr time_t second_before_midnight =
        convertRawTimeStringToTimeOffset("23:59:59").value();
    constexpr time_t midnight =
        convertRawTimeStringToTimeOffset("00:00:00").value();

    sleepTime = std::chrono::seconds((second_before_midnight - now) + 1 +
                                     (later - midnight)) -
                eventDuration;
  }

  return std::clamp(sleepTime, std::chrono::seconds(0),
                    std::chrono::seconds(TWENTY_FOUR_HOURS));
}

} // namespace _helper

DynamicBackgroundData::DynamicBackgroundData(
    std::filesystem::path dataDirectory, BackgroundSetMode mode,
    std::optional<TransitionInfo> transition, BackgroundSetOrder order,
    std::vector<std::string> imageNames, std::vector<time_t> times)
    : dataDirectory(std::move(dataDirectory)), mode(mode),
      transition(transition), order(order), imageNames(std::move(imageNames)),
      times(std::move(times)) {}
} // namespace dynamic_paper
