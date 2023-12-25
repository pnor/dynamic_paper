#include "dynamic_background_set.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <format>
#include <limits>
#include <random>
#include <ranges>
#include <thread>

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

// TODO move this into config? (per background)
const int NUM_TRANSITION_STEPS = 10;

// ===== Helper Objects ===============
/** Information needed for the event to change the background to an image */
struct SetBackgroundEvent {
  const std::filesystem::path imagePath;

  explicit SetBackgroundEvent(const std::filesystem::path &imagePath)
      : imagePath(imagePath) {}
};

/** Information needed for the event to gradually interpolate between one image
 * and the next */
struct LerpBackgroundEvent {
  const std::filesystem::path commonImageDirectory;
  const std::string startImageName;
  const std::string endImageName;
  const unsigned int duration;
  const unsigned int numSteps;

  LerpBackgroundEvent(const std::filesystem::path &commonImageDirectory,
                      const std::string &startImageName,
                      const std::string &endImageName,
                      const unsigned int duration, const unsigned int numSteps)
      : commonImageDirectory(commonImageDirectory),
        startImageName(startImageName), endImageName(endImageName),
        duration(duration), numSteps(numSteps) {}
};

/**
 * Class to be used by `std::shuffle` that uses `std::rand`.
 * Mainly for the ability to control the random order by calling `std::srand`
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

inline unsigned int chooseRandomSeed() {
  std::random_device rd;
  std::mt19937 g(rd());

  using limits = std::numeric_limits<unsigned int>;

  std::uniform_int_distribution<unsigned int> uniformDist(limits::min(),
                                                          limits::max());
  return uniformDist(g);
}

/**
 * Shuffles a vector using `std::rand()` as the source of randomness
 */
template <typename T> static void shuffleVector(std::vector<T> &vec) {
  std::shuffle(vec.begin(), vec.end(), RandUniformRandomBitGenerator());
}

static inline bool eventLsitIsSortedByTime(const EventList &eventList) {
  for (size_t i = 0; i < eventList.size() - 1; i++) {
    if (!(eventList[i].first < eventList[i + 1].first)) {
      return false;
    }
  }
  return true;
}

static time_t getCurrentTime() {
  const std::string timeString =
      std::format("{:%T}", std::chrono::floor<std::chrono::seconds>(
                               std::chrono::system_clock::now()));
  std::optional<time_t> optTime =
      convertRawTimeStringToTimeOffset(timeString.substr(0, 5));

  logAssert(optTime.has_value(), "Unable to parse valid time from return "
                                 "result of current time as string");

  return optTime.value();
}

static SetBackgroundEvent *getBackgroundEventFromVariant(Event &event) {
  std::optional<SetBackgroundEvent *> optEvent =
      std::get_if<SetBackgroundEvent>(&event);
  logAssert(optEvent.has_value(),
            "event variant must have SetBackgroundEvent to use this function");
  return optEvent.value();
}

static time_t calculateLerpEventTime(const unsigned int transitionDuration,
                                     const time_t nextEventTime,
                                     const int numberSteps) {
  const unsigned int offset =
      std::max(1u, transitionDuration / static_cast<unsigned int>(numberSteps));
  return nextEventTime - offset;
}

/**
 * Returns the times and names in `dynamicData` sorted by time.
 * Preserves the pair relation between the names and times.
 * Example: if a time was at index 1 in times and a name was at index 1 in
 * names, they would appear in the same pair in the sorted output.
 **/
static std::vector<std::pair<time_t, std::string>>
timesAndNamesSortedByTime(const DynamicBackgroundData *dynamicData) {
  std::vector<std::pair<time_t, std::string>> timesNames;

  for (const auto &timeName :
       std::views::zip(dynamicData->times, dynamicData->imageNames)) {
    timesNames.push_back(timeName);
  }

  std::ranges::sort(timesNames, {}, &std::pair<time_t, std::string>::first);

  return timesNames;
}

/**
 * Returns the times and names in `dynamicData` sorted by time, but with the
 * image name chosen randomly.
 */
static std::vector<std::pair<time_t, std::string>>
timesAndRandomNamesSortedByTime(const DynamicBackgroundData *dynamicData) {
  std::vector<std::pair<time_t, std::string>> timesAndNames;

  std::vector<std::string> names = dynamicData->imageNames;
  shuffleVector(names);

  for (size_t i = 0; i < dynamicData->times.size(); i++) {
    timesAndNames.emplace_back(dynamicData->times[i], names[i % names.size()]);
  }

  return timesAndNames;
}

static EventList createEventListFromTimesAndNames(
    const DynamicBackgroundData *dynamicData,
    const std::vector<std::pair<time_t, std::string>> &timesAndNames) {
  EventList eventList;

  logAssert(timesAndNames.size() >= 1, "Times and names cannot be empty");

  eventList.emplace_back(
      timesAndNames[0].first,
      SetBackgroundEvent(dynamicData->dataDirectory / timesAndNames[0].second));

  for (EventList::size_type i = 1; i < eventList.size(); i++) {
    // transition event
    if (dynamicData->transitionDuration.has_value()) {
      const LerpBackgroundEvent lerpEvent(
          dynamicData->dataDirectory, timesAndNames[i - 1].second,
          timesAndNames[i].second, dynamicData->transitionDuration.value(),
          NUM_TRANSITION_STEPS);

      const time_t transitionTime =
          timesAndNames[i].first - dynamicData->transitionDuration.value();

      eventList.emplace_back(transitionTime, lerpEvent);
    }
    // set background event
    eventList.emplace_back(timesAndNames[i].first,
                           SetBackgroundEvent(dynamicData->dataDirectory /
                                              timesAndNames[i].second));
  }

  return eventList;
}

static EventList getEventList(const DynamicBackgroundData *dynamicData) {
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

static std::pair<TimeAndEvent, time_t>
getCurrentEventAndNextTime(const EventList &eventList, const time_t time) {
  logAssert(eventList.size() >= 1, "Event list is empty");

  auto firstAfterTime =
      std::ranges::find_if(eventList, [time](const TimeAndEvent &timeAndEvent) {
        return timeAndEvent.first > time;
      });

  if (firstAfterTime == eventList.begin() ||
      firstAfterTime == eventList.end()) {
    TimeAndEvent current = eventList.back();
    time_t next = eventList.front().first;

    return std::make_pair(current, next);
  } else {
    TimeAndEvent current = *(firstAfterTime - 1);
    time_t next = firstAfterTime->first;

    return std::make_pair(current, next);
  }
}

static void doEvent(const Event &event,
                    const DynamicBackgroundData *backgroundData,
                    const Config &config) {
  std::visit(
      overloaded{
          [&config, backgroundData](const SetBackgroundEvent &event) {
            std::expected<void, BackgroundError> result =
                setBackgroundToImage(event.imagePath, backgroundData->mode,
                                     config.backgroundSetterMethod);

            if (!result.has_value()) {
              logError("Error occured when trying to set background");
            }

            if (config.hookScript.has_value()) {
              runHookCommand(config.hookScript.value(), event.imagePath);
            }
          },
          [&config, backgroundData](const LerpBackgroundEvent &event) {
            std::expected<void, BackgroundError> result =
                lerpBackgroundBetweenImages(
                    event.commonImageDirectory, event.startImageName,
                    event.endImageName, event.duration, event.numSteps,
                    backgroundData->mode, config.backgroundSetterMethod);

            if (!result.has_value()) {
              logError(
                  "Error occured when trying to interpolate the background");
            }
          }},
      event);
}

static void sleepUntilNextTime(const time_t &now, const time_t &later) {
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

  logDebug(std::format("Sleeping for {} seconds ...", sleepTime));
  std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
}

// ===== Main Loop Logic ===============

void doBackgroundLoop(const DynamicBackgroundData *backgroundData,
                      const Config &config) {
  const unsigned int seed = chooseRandomSeed();

  while (keepRunningBackgroundLoop) {

    // Reset the random seed on each iteration of the loop to ensure the order
    // of `random` dynamic backgrounds is consistent between each reconstruction
    // of the event list.
    std::srand(seed);

    const time_t currentTime = getCurrentTime();

    EventList eventList = getEventList(backgroundData);
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
    std::optional<unsigned int> transitionDuration, BackgroundSetOrder order,
    std::vector<std::string> imageNames, std::vector<time_t> times)
    : dataDirectory(dataDirectory), mode(mode),
      transitionDuration(transitionDuration), order(order),
      imageNames(imageNames), times(times) {}

void DynamicBackgroundData::show(const BackgroundSetterMethod &method,
                                 const Config &config) const {
  logInfo("Show dynamic background");

  doBackgroundLoop(this, config);
}

} // namespace dynamic_paper
