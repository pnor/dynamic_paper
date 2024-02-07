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

EventList createEventListFromTimesAndNames(
    const DynamicBackgroundData *dynamicData,
    const std::vector<std::pair<time_t, std::string>> &timesAndNames) {
  EventList eventList;

  logAssert(!timesAndNames.empty(), "Times and names cannot be empty");

  eventList.emplace_back(
      timesAndNames[0].first,
      SetBackgroundEvent(dynamicData->dataDirectory / timesAndNames[0].second));

  for (EventList::size_type i = 1; i < eventList.size(); i++) {
    // transition event
    if (dynamicData->transition.has_value()) {
      const LerpBackgroundEvent lerpEvent(
          dynamicData->dataDirectory, timesAndNames[i - 1].second,
          timesAndNames[i].second, dynamicData->transition->duration,
          dynamicData->transition->steps);

      const time_t transitionTime =
          timesAndNames[i].first - dynamicData->transition->duration.count();

      eventList.emplace_back(transitionTime, lerpEvent);
    }
    // set background event
    eventList.emplace_back(timesAndNames[i].first,
                           SetBackgroundEvent(dynamicData->dataDirectory /
                                              timesAndNames[i].second));
  }

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

std::chrono::seconds timeUntilNext(const time_t &now, const time_t &later) {
  std::chrono::seconds sleepTime;

  if (now < later) {
    sleepTime = std::chrono::seconds(later - now);
  } else { // Sleep past a day boundary / now >= later
    constexpr time_t second_before_midnight =
        convertRawTimeStringToTimeOffset("23:59:59").value();
    constexpr time_t midnight =
        convertRawTimeStringToTimeOffset("00:00:00").value();

    sleepTime = std::chrono::seconds((second_before_midnight - now) + 1 +
                                     (later - midnight));
  }

  return sleepTime;
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
