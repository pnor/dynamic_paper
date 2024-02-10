#pragma once

#include <chrono>
#include <filesystem>
#include <optional>
#include <vector>

#include "background_set_enums.hpp"
#include "background_setter.hpp"
#include "config.hpp"
#include "time_util.hpp"
#include "variant_visitor_templ.hpp"

/** Dynamic Background sets change wallpaper depending on the time of day. They
 * sleep until the next time to show a wallpaper is hit
 */

namespace dynamic_paper {

struct TransitionInfo {
  std::chrono::seconds duration;
  unsigned int steps;

  explicit TransitionInfo(unsigned int duration, unsigned int steps)
      : duration(duration), steps(steps) {}
};

/** Type of `BackgroundSet` that shows different wallpapers at different times,
 * and changes over time*/
struct DynamicBackgroundData {
  std::filesystem::path dataDirectory;
  BackgroundSetMode mode;
  /** nullopt if does not transition. */
  std::optional<TransitionInfo> transition;

  BackgroundSetOrder order;
  std::vector<std::string> imageNames;
  /** each entry represents number seconds after 00:00 to do a transition */
  std::vector<time_t> times;

  DynamicBackgroundData(std::filesystem::path dataDirectory,
                        BackgroundSetMode mode,
                        std::optional<TransitionInfo> transition,
                        BackgroundSetOrder order,
                        std::vector<std::string> imageNames,
                        std::vector<time_t> times);

  /** Updates the background shown for the current time, and returns the amount
   * of seconds until the next event will be shown. */
  template <CanSetBackgroundTrait T>
  [[nodiscard]] std::chrono::seconds
  updateBackground(time_t currentTime, const Config &config,
                   T &&backgroundSetterFunc) const;
};

// ===== Header Helper ===============

namespace _helper {

// --- Types ---

struct SetBackgroundEvent;
struct LerpBackgroundEvent;

using Event = std::variant<SetBackgroundEvent, LerpBackgroundEvent>;
using TimeAndEvent = std::pair<time_t, Event>;
using EventList = std::vector<TimeAndEvent>;

// --- Structs ---

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

// --- Helper Function Declarations ---

/** Describes `error` in a log message */
void describeError(BackgroundError error);

/** Returns number of seconds until `later`, assuming the time is `now` */
std::chrono::seconds timeUntilNext(const time_t &now, const time_t &later);

EventList getEventList(const DynamicBackgroundData *dynamicData);

std::pair<TimeAndEvent, time_t>
getCurrentEventAndNextTime(const EventList &eventList, time_t time);

bool eventListIsSortedByTime(const EventList &eventList);

unsigned int chooseRandomSeed();

// --- Event Processing ---

template <CanSetBackgroundTrait T>
void doEvent(const Event &event, const DynamicBackgroundData *backgroundData,
             const Config &config, T &&backgroundSetterFunc) {
  std::visit(
      overloaded{[&config, backgroundData,
                  backgroundSetterFunc](const SetBackgroundEvent &event) {
                   tl::expected<void, BackgroundError> result =
                       backgroundSetterFunc(event.imagePath,
                                            backgroundData->mode,
                                            config.backgroundSetterMethod);

                   if (!result.has_value()) {
                     describeError(result.error());
                   }

                   if (config.hookScript.has_value()) {
                     runHookCommand(config.hookScript.value(), event.imagePath);
                   }
                 },
                 [&config, backgroundData,
                  backgroundSetterFunc](const LerpBackgroundEvent &event) {
                   tl::expected<void, BackgroundError> result =
                       lerpBackgroundBetweenImages<T>(
                           event.commonImageDirectory, event.startImageName,
                           event.endImageName, config.imageCacheDirectory,
                           event.duration, event.numSteps, backgroundData->mode,
                           config.backgroundSetterMethod, backgroundSetterFunc);

                   if (!result.has_value()) {
                     describeError(result.error());
                   }
                 }},
      event);
}

// --- Main Loop Logic ---

template <CanSetBackgroundTrait T>
std::chrono::seconds updateBackgroundAndReturnTimeTillNext(
    const time_t currentTime, const DynamicBackgroundData *backgroundData,
    const Config &config, const unsigned int seed, T &&backgroundSetterFunc) {
  // Reset the random seed on each iteration of the loop to ensure the order
  // of `random` dynamic backgrounds is consistent between each reconstruction
  // of the event list.
  std::srand(seed);

  const EventList eventList = getEventList(backgroundData);
  logAssert(eventListIsSortedByTime(eventList),
            "Event list is not sorted by time from earliest to latest");

  const std::pair<TimeAndEvent, time_t> currentEventAndNextTime =
      getCurrentEventAndNextTime(eventList, currentTime);

  const Event &currentEvent = currentEventAndNextTime.first.second;

  _helper::doEvent<T>(currentEvent, backgroundData, config,
                      backgroundSetterFunc);

  const time_t &nextTime = currentEventAndNextTime.second;

  return timeUntilNext(currentTime, nextTime);
}

} // namespace _helper

// ===== Definition =====

template <CanSetBackgroundTrait T>
[[nodiscard]] std::chrono::seconds
DynamicBackgroundData::updateBackground(const time_t currentTime,
                                        const Config &config,
                                        T &&backgroundSetterFunc) const {
  logTrace("Show dynamic background");

  const unsigned int seed = _helper::chooseRandomSeed();
  logTrace("Random seed is {}", seed);

  return _helper::updateBackgroundAndReturnTimeTillNext<T>(
      currentTime, this, config, seed, backgroundSetterFunc);
}

} // namespace dynamic_paper
