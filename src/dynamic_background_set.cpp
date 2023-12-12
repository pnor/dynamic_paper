#include "dynamic_background_set.hpp"

#include <algorithm>
#include <chrono>
#include <format>
#include <random>
#include <ranges>

#include "time_util.hpp"

namespace dynamic_paper {
using std::move;

struct SetBackgroundEvent;
struct LerpBackgroundEvent;
using Event = std::variant<SetBackgroundEvent, LerpBackgroundEvent>;
using TimeAndEvent = std::pair<time_t, Event>;
using EventList = std::vector<TimeAndEvent>;

// TODO move this into config?
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
  const std::filesystem::path startImagePath;
  const std::filesystem::path endImagePath;
  const unsigned int duration;
  const unsigned int numSteps;

  LerpBackgroundEvent(const std::filesystem::path &start,
                      const std::filesystem::path &end,
                      const unsigned int duration, const unsigned int numSteps)
      : startImagePath(start), endImagePath(end), duration(duration),
        numSteps(numSteps) {}
};

// ===== Helper ===============

template <typename T> static void shuffleVector(std::vector<T> &vec) {
  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(vec.begin(), vec.end(), g);
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

  std::random_device rd;
  std::mt19937 g(rd());
  std::uniform_int_distribution<int> randomIndexGenerator(
      0, dynamicData->imageNames.size() - 1);

  for (const time_t &time : dynamicData->times) {
    const std::string randomImageName =
  }
}

static EventList getLinearEventList(const DynamicBackgroundData *dynamicData) {
  EventList eventList;

  std::vector<std::pair<time_t, std::string>> timesAndNames =
      timesAndNamesSortedByTime(dynamicData);

  logAssert(timesAndNames.size() >= 1, "Times and names cannot be empty");

  eventList.emplace_back(
      timesAndNames[0].first,
      SetBackgroundEvent(dynamicData->dataDirectory / timesAndNames[0].second));

  for (EventList::size_type i = 1; i < eventList.size(); i++) {
    // transition event
    if (dynamicData->transitionDuration.has_value()) {
      const std::filesystem::path beforePath =
          dynamicData->dataDirectory / timesAndNames[i - 1].second;
      const std::filesystem::path afterPath =
          dynamicData->dataDirectory / timesAndNames[i].second;

      const LerpBackgroundEvent lerpEvent(
          beforePath, afterPath, dynamicData->transitionDuration.value(),
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

static EventList getRandomEventList(const DynamicBackgroundData *dynamicData) {
  EventList eventList;

  std::vector<std::string> imageNames(dynamicData->imageNames);
  shuffleVector(imageNames);
}

static void
insertRandomSetBackgroundEvents(EventList &eventList,
                                const DynamicBackgroundData *dynamicData) {
  std::vector<std::string> imageNamesCopy(dynamicData->imageNames);
  shuffleVector(imageNamesCopy);

  for (size_t i = 0; i < dynamicData->times.size(); i++) {
    const Event event = SetBackgroundEvent(
        dynamicData->dataDirectory / imageNamesCopy[i % imageNamesCopy.size()]);
    eventList.emplace_back(dynamicData->times[i], event);
  }
}

static void insertTransitionEvents(EventList &eventList,
                                   const DynamicBackgroundData *dynamicData) {
  logAssert(dynamicData->transitionDuration.has_value(),
            "Need a transition duration to insert transition events");

  const unsigned int transitionDuration =

      dynamicData->transitionDuration.value();

  for (EventList::size_type i = 0; i < eventList.size() - 1; i += 2) {
    const SetBackgroundEvent *beforeEvent =
        getBackgroundEventFromVariant(eventList[0].second);
    const SetBackgroundEvent *afterEvent =
        getBackgroundEventFromVariant(eventList[1].second);

    const time_t afterEventTime = eventList[1].first;

    const LerpBackgroundEvent lerpEvent(
        beforeEvent->imagePath, afterEvent->imagePath, transitionDuration,
        NUM_TRANSITION_STEPS);

    const time_t lerpEventTime = calculateLerpEventTime(
        transitionDuration, afterEventTime, NUM_TRANSITION_STEPS);
    const Event lerpEventVariant(lerpEvent);

    const std::pair<time_t, Event> timeEvent =
        std::make_pair(lerpEventTime, lerpEventVariant);
    eventList.insert(eventList.begin() + i + 1, timeEvent);
  }
}

static EventList getEventList(const DynamicBackgroundData *dynamicData) {
  EventList eventList;

  switch (dynamicData->order) {
  case BackgroundSetOrder::Linear: {
    insertLinearSetBackgroundEvents(eventList, dynamicData);
    break;
  }
  case BackgroundSetOrder::Random: {
    insertRandomSetBackgroundEvents(eventList, dynamicData);
    break;
  }
  }

  if (dynamicData->transitionDuration.has_value()) {
    insertTransitionEvents(eventList, dynamicData);
  }

  return eventList;
}

static std::pair<TimeAndEvent, TimeAndEvent> &
getCurrentAndNextTimeAndEvent(const EventList &eventList, const time_t time) {
  logAssert(eventList.size() >= 1, "Event list is empty");
  throw std::logic_error("");
}

// ===== Main Loop Logic ===============

void doBackgroundLoop(const DynamicBackgroundData *backgroundData,
                      const Config &config) {
  // TODO
  // 1. get current time
  const time_t currentTime = getCurrentTime();
  // 2. do the last thing before current time
  // - its either (set background to X) or (lerp between X and Y for z seconds)
  EventList eventList = getEventList(backgroundData);
  logAssert(eventLsitIsSortedByTime(eventList),
            "Event list is not sorted by time from earliest to latest");
  std::pair<TimeAndEvent, TimeAndEvent> currentAndNextTimeAndEvent =
      getCurrentAndNextTimeAndEvent(eventList, currentTime);
  // 3. wait until next time
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
  while (true) {
    doBackgroundLoop(this, config);
  }
}

} // namespace dynamic_paper
