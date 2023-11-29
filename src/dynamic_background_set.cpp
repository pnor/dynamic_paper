#include "dynamic_background_set.hpp"

#include <algorithm>
#include <chrono>
#include <format>
#include <ranges>

#include "time_util.hpp"

namespace dynamic_paper {

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

using EventData = std::variant<SetBackgroundEvent, LerpBackgroundEvent>;

// ===== Helper ===============

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

static SetBackgroundEvent *getBackgroundEventFromVariant(EventData &event) {
  std::optional<SetBackgroundEvent *> optEvent =
      std::get_if<SetBackgroundEvent>(&event);
  logAssert(optEvent.has_value(),
            "event variant must have SetBackgroundEvent to use this function");
  return optEvent.value();
}

static time_t calculateLerpEventTime(const unsigned int transitionDuration,
                                     const time_t nextEvenTime,
                                     const int numberSteps) {
  const unsigned int offset =
      std::max(1, transitionDuration / static_cast<unsigned int>(numberSteps));
  return nextEvenTime - offset;
}

static void
insertTransitionEvents(std::vector<std::pair<time_t, EventData>> &eventList,
                       const DynamicBackgroundData &dynamicData) {
  logAssert(dynamicData.transitionDuration.has_value(),
            "Need a transition duration to insert transition events");

  const unsigned int transitionDuration =
      dynamicData.transitionDuration.value();

  for (std::vector<std::pair<time_t, EventData>>::size_type i = 0;
       i < eventList.size() - 1; i++) {
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
    const EventData lerpEventVariant(lerpEvent);

    eventList.emplace(eventList.begin() + i + 1, lerpEventTime,
                      lerpEventVariant);
  }
}

static std::vector<std::pair<time_t, EventData>>
getEventList(const DynamicBackgroundData &dynamicData) {
  std::vector<std::pair<time_t, EventData>> eventList;

  switch (dynamicData.order) {
  case BackgroundSetOrder::Linear: {
    for (const auto &imageTime :
         std::views::zip(dynamicData.imageNames, dynamicData.times)) {
      const time_t &time = std::get<1>(imageTime);
      const EventData event = SetBackgroundEvent(dynamicData.dataDirectory /
                                                 std::get<0>(imageTime));

      eventList.emplace_back(time, event);
    }

    break;
  }
  case BackgroundSetOrder::Random: {
    // TODO
    break;
  }
  }

  return eventList;
}

// ===== Main Loop Logic ===============

void doBackgroundLoop(const DynamicBackgroundData *backgroundData,
                      const Config &config) {
  // TODO
  // 1. get current time
  const time_t currentTime = getCurrentTime();
  // 2. do the last thing before current time
  // - its either (set background to X) or (lerp between X and Y for z seconds)
  //
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
