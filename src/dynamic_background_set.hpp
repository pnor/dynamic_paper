#pragma once

/**
 * Dynamic Background sets change wallpaper depending on the time of day. They
 * sleep until the next time to show a wallpaper is hit
 */

#include <chrono>
#include <filesystem>
#include <optional>
#include <vector>

#include "background_set_enums.hpp"
#include "background_setter_definition.hpp"
#include "config.hpp"
#include "script_executor.hpp"
#include "time_from_midnight.hpp"
#include "transition_info.hpp"
#include "variant_visitor_templ.hpp"

namespace dynamic_paper {

/** Type of `BackgroundSet` that shows different wallpapers at different times,
 * and changes over time*/
struct DynamicBackgroundData {

  std::filesystem::path imageDirectory;

  BackgroundSetMode mode;

  /** nullopt if does not transition. */
  std::optional<TransitionInfo> transition;

  BackgroundSetOrder order;

  std::vector<std::string> imageNames;

  /** each entry represents number seconds after 00:00 to do a transition */
  std::vector<TimeFromMidnight> times;

  DynamicBackgroundData(std::filesystem::path imageDirectory,
                        BackgroundSetMode mode,
                        std::optional<TransitionInfo> transition,
                        BackgroundSetOrder order,
                        std::vector<std::string> imageNames,
                        std::vector<TimeFromMidnight> times);

  /** Updates the background shown for the current time, and returns the amount
   * of seconds until the next event will be shown. */
  template <CanSetBackgroundTrait T,
            ChangesFilesystem Files = FilesystemHandler,
            GetsCompositeImages CompositeImages = ImageCompositor>
  [[nodiscard]] std::chrono::seconds
  updateBackground(TimeFromMidnight currentTime, const Config &config,
                   T &&backgroundSetFunction) const;
};

// ===== Header Helper ===============

namespace detail {

// --- Types ---

struct SetBackgroundEvent;
struct LerpBackgroundEvent;

using Event = std::variant<SetBackgroundEvent, LerpBackgroundEvent>;
using TimeAndEvent = std::pair<TimeFromMidnight, Event>;
using EventList = std::vector<TimeAndEvent>;

// --- Structs ---

/** Information needed for the event to change the background to an image */
struct SetBackgroundEvent {
  std::filesystem::path imagePath;
};

/** Information needed for the event to gradually interpolate between one image
 * and the next */
struct LerpBackgroundEvent {
  std::filesystem::path commonImageDirectory;
  std::string startImageName;
  std::string endImageName;
  TransitionInfo transition;
};

// --- Helper Function Declarations ---

/** Describes `error` in a log message */
void describeError(BackgroundError error);

/** Returns number of seconds until `later`, assuming the time is `now` */
std::chrono::seconds timeUntilNext(const TimeFromMidnight &now,
                                   std::chrono::seconds eventDuration,
                                   const TimeFromMidnight &later);

/** Gets the list of events to do over the course of the day, sorted by time */
EventList getEventList(const DynamicBackgroundData *dynamicData);

/** Returns the amount of time an event takes */
std::chrono::seconds getEventDuration(const Event &event);

std::pair<TimeAndEvent, TimeFromMidnight>
getCurrentEventAndNextTime(const EventList &eventList, TimeFromMidnight time);

bool eventListIsSortedByTime(const EventList &eventList);

unsigned int chooseRandomSeed();

/** Logs out an easily readable version of the event list */
void logPrintEventList(const EventList &eventList);

// --- Event Processing ---

template <CanSetBackgroundTrait T, ChangesFilesystem Files,
          GetsCompositeImages CompositeImages>
void doEvent(const Event &event, const DynamicBackgroundData *backgroundData,
             const Config &config, T &&backgroundSetFunction) {
  std::visit(
    overloaded{
    [&config, backgroundData, &backgroundSetFunction](const SetBackgroundEvent &event) {
      std::forward<T>(backgroundSetFunction)(event.imagePath, backgroundData->mode);

      logTrace("Did Set background event, set to {}",
               event.imagePath.string());

      if (config.hookScript.has_value()) {
        tl::expected<void, ScriptError> hookResult =
          runHookScript(config.hookScript.value(), event.imagePath);

        if (!hookResult.has_value()) {
          logError("Error occured relating to forking when running "
                   "hook script");
        }
      }
    },
    [&config, backgroundData,
     &backgroundSetFunction](const LerpBackgroundEvent &event) {
      // std::decay_t<T> func = std::forward<T>(backgroundSetFunction);

      logTrace("About to start lerping background");

      tl::expected<void, BackgroundError> result =
        lerpBackgroundBetweenImages<std::decay_t<T>, Files,
                                    CompositeImages>(
                                      event.commonImageDirectory, event.startImageName,
                                      event.endImageName, config.imageCacheDirectory,
                                      event.transition, backgroundData->mode, std::move(std::forward<T>(backgroundSetFunction)));

      if (!result.has_value()) {
        describeError(result.error());
      }
    }},
    event);
}

// --- Main Loop Logic ---

template <CanSetBackgroundTrait T, ChangesFilesystem Files,
          GetsCompositeImages CompositeImages>
std::chrono::seconds updateBackgroundAndReturnTimeTillNext(
    const TimeFromMidnight currentTime,
    const DynamicBackgroundData *backgroundData, const Config &config,
    const unsigned int seed, T &&backgroundSetFunction) {

  // Reset the random seed on each iteration of the loop to ensure the order
  // of `random` dynamic backgrounds is consistent between each reconstruction
  // of the event list.
  std::srand(seed);

  const EventList eventList = getEventList(backgroundData);

  detail::logPrintEventList(eventList);

  logAssert(eventListIsSortedByTime(eventList),
            "Event list is not sorted by time from earliest to latest");

  const std::pair<TimeAndEvent, TimeFromMidnight> currentEventAndNextTime =
      getCurrentEventAndNextTime(eventList, currentTime);

  const Event &currentEvent = currentEventAndNextTime.first.second;
  logTrace("Doing Current Event for time {}",
           currentEventAndNextTime.first.first);

  detail::doEvent<T, Files, CompositeImages>(
      currentEvent, backgroundData, config,
      std::forward<T>(backgroundSetFunction));

  const std::chrono::seconds currentEventDuration =
      getEventDuration(currentEvent);

  const TimeFromMidnight &nextTime = currentEventAndNextTime.second;

  return timeUntilNext(currentTime, currentEventDuration, nextTime);
}

} // namespace detail

// ===== Definition =====

template <CanSetBackgroundTrait T, ChangesFilesystem Files,
          GetsCompositeImages CompositeImages>
[[nodiscard]] std::chrono::seconds
DynamicBackgroundData::updateBackground(const TimeFromMidnight currentTime,
                                        const Config &config,
                                        T &&backgroundSetFunction) const {
  logTrace("Show dynamic background");

  const unsigned int seed = detail::chooseRandomSeed();
  logTrace("Random seed is {}", seed);

  return detail::updateBackgroundAndReturnTimeTillNext<T, Files,
                                                       CompositeImages>(
      currentTime, this, config, seed, std::forward<T>(backgroundSetFunction));
}

} // namespace dynamic_paper
