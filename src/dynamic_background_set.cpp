#include "dynamic_background_set.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <format>
#include <limits>
#include <random>
#include <ranges>
#include <thread>
#include <utility>

#include "background_setter.hpp"
#include "globals.hpp"
#include "time_util.hpp"
#include "variant_visitor_templ.hpp"

namespace dynamic_paper {

struct SetBackgroundEvent;
struct LerpBackgroundEvent;

using Event = std::variant<SetBackgroundEvent, LerpBackgroundEvent>;
using TimeAndEvent = std::pair<time_t, Event>;
using EventList = std::vector<TimeAndEvent>;

// ===== Helper Objects ===============
/** Information needed for the event to change the background to an image */
struct SetBackgroundEvent {
  std::filesystem::path imagePath;

  explicit SetBackgroundEvent(std::filesystem::path imagePath)
      : imagePath(std::move(imagePath)) {}
};

/** Information needed for the event to gradually interpolate between one image
 * and the next */
struct LerpBackgroundEvent {
  std::filesystem::path commonImageDirectory;
  std::string startImageName;
  std::string endImageName;
  std::chrono::seconds duration;
  unsigned int numSteps;

  LerpBackgroundEvent(std::filesystem::path commonImageDirectory,
                      std::string startImageName, std::string endImageName,
                      const std::chrono::seconds duration,
                      const unsigned int numSteps)
      : commonImageDirectory(std::move(commonImageDirectory)),
        startImageName(std::move(startImageName)),
        endImageName(std::move(endImageName)), duration(duration),
        numSteps(numSteps) {}
};

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

// ===== Helper ===============

namespace {

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

inline unsigned int chooseRandomSeed() {
  std::random_device random_device;
  std::mt19937 generator(random_device());

  using limits = std::numeric_limits<unsigned int>;

  std::uniform_int_distribution<unsigned int> uniformDist(limits::min(),
                                                          limits::max());
  return uniformDist(generator);
}

/**
 * Shuffles a vector using `std::rand()` as the source of randomness
 */
template <typename T> void shuffleVector(std::vector<T> &vec) {
  std::shuffle(vec.begin(), vec.end(), RandUniformRandomBitGenerator());
}

inline bool eventLsitIsSortedByTime(const EventList &eventList) {
  for (size_t i = 0; i < eventList.size() - 1; i++) {
    if (!(eventList[i].first < eventList[i + 1].first)) {
      return false;
    }
  }
  return true;
}

time_t getCurrentTime() {
  // HH:MM
  constexpr size_t HOURS_MINUTES_SIZE = 5;

  const std::string timeString =
      std::format("{:%T}", std::chrono::floor<std::chrono::seconds>(
                               std::chrono::system_clock::now()));
  std::optional<time_t> optTime = convertRawTimeStringToTimeOffset(
      timeString.substr(0, HOURS_MINUTES_SIZE));

  logAssert(optTime.has_value(), "Unable to parse valid time from return "
                                 "result of current time as string");

  return optTime.value();
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

void doEvent(const Event &event, const DynamicBackgroundData *backgroundData,
             const Config &config) {
  std::visit(
      overloaded{[&config, backgroundData](const SetBackgroundEvent &event) {
                   tl::expected<void, BackgroundError> result =
                       setBackgroundToImage(event.imagePath,
                                            backgroundData->mode,
                                            config.backgroundSetterMethod);

                   if (!result.has_value()) {
                     describeError(result.error());
                   }

                   if (config.hookScript.has_value()) {
                     runHookCommand(config.hookScript.value(), event.imagePath);
                   }
                 },
                 [&config, backgroundData](const LerpBackgroundEvent &event) {
                   tl::expected<void, BackgroundError> result =
                       lerpBackgroundBetweenImages(
                           event.commonImageDirectory, event.startImageName,
                           event.endImageName, config.imageCacheDirectory,
                           event.duration, event.numSteps, backgroundData->mode,
                           config.backgroundSetterMethod);

                   if (!result.has_value()) {
                     describeError(result.error());
                   }
                 }},
      event);
}

void sleepUntilNextTime(const time_t &now, const time_t &later) {
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

  logDebug("Sleeping for {} seconds ...", sleepTime);
  std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
}

} // namespace

// ===== Main Loop Logic ===============

void doBackgroundLoop(const DynamicBackgroundData *backgroundData,
                      const Config &config) {
  const unsigned int seed = chooseRandomSeed();

  while (true) {

    // Reset the random seed on each iteration of the loop to ensure the order
    // of `random` dynamic backgrounds is consistent between each reconstruction
    // of the event list.
    std::srand(seed);

    const time_t currentTime = getCurrentTime();

    const EventList eventList = getEventList(backgroundData);
    logAssert(eventLsitIsSortedByTime(eventList),
              "Event list is not sorted by time from earliest to latest");

    std::pair<TimeAndEvent, time_t> currentEventAndNextTime =
        getCurrentEventAndNextTime(eventList, currentTime);

    const Event &currentEvent = currentEventAndNextTime.first.second;

    doEvent(currentEvent, backgroundData, config);

    const time_t &nextTime = currentEventAndNextTime.second;

    sleepUntilNextTime(currentTime, nextTime);
  }
}

// ===== Header ===============

DynamicBackgroundData::DynamicBackgroundData(
    std::filesystem::path dataDirectory, BackgroundSetMode mode,
    std::optional<TransitionInfo> transition, BackgroundSetOrder order,
    std::vector<std::string> imageNames, std::vector<time_t> times)
    : dataDirectory(std::move(dataDirectory)), mode(mode),
      transition(transition), order(order), imageNames(std::move(imageNames)),
      times(std::move(times)) {}

void DynamicBackgroundData::show(const Config &config) const {
  logTrace("Show dynamic background");

  doBackgroundLoop(this, config);
}

} // namespace dynamic_paper
