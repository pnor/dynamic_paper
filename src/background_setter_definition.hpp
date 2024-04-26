#pragma once

/**
 * Definitions for templated functions in background_setter.hpp
 *
 * Definitions are a bit involved making files that include
 * background_setter.hpp just for class names much slower to compile
 */

#include "background_setter.hpp"

#include "time_util.hpp"
#include "transition_info.hpp"

namespace dynamic_paper {

template <CanSetBackgroundTrait T, ChangesFilesystem Files,
          GetsCompositeImages CompositeImages>
tl::expected<void, BackgroundError> lerpBackgroundBetweenImages(
    const std::filesystem::path &commonImageDirectory,
    const std::string &beforeImageName, const std::string &afterImageName,
    const std::filesystem::path &cacheDirectory,
    const TransitionInfo &transition, const BackgroundSetMode mode,
    T backgroundSetFunction) {

  const bool dirCreationResult =
      Files::createDirectoryIfDoesntExist(cacheDirectory);
  if (!dirCreationResult) {
    return tl::make_unexpected(BackgroundError::NoCacheDir);
  }

  const unsigned int denominator = transition.steps + 1;
  for (unsigned int i = 0; i < transition.steps; i++) {
    std::optional<tl::expected<void, BackgroundError>> potentialError =
        std::nullopt;

    std::chrono::milliseconds timeElapsed =
        timeToRunCodeBlock([i, denominator, &commonImageDirectory,
                            &beforeImageName, &afterImageName, &cacheDirectory,
                            mode, backgroundSetFunction, &potentialError]() {
          // modifies `i` so is more in the middle of the range, to avoid
          // interpolating 0% and 100% images
          const unsigned int numerator = i + 1;

          const float percentageFloat =
              static_cast<float>(numerator) / static_cast<float>(denominator);
          const unsigned int percentage = std::clamp(
              static_cast<unsigned int>(percentageFloat * 100.0F), 0U, 100U);

          const tl::expected<std::filesystem::path, CompositeImageError>
              expectedCompositedImage = CompositeImages::getCompositedImage(
                  commonImageDirectory, beforeImageName, afterImageName,
                  cacheDirectory, percentage);

          if (!expectedCompositedImage.has_value()) {
            potentialError =
                tl::unexpected(BackgroundError::CompositeImageError);
          }

          backgroundSetFunction(expectedCompositedImage.value(), mode);

          logTrace("Interpolating to {}...",
                   expectedCompositedImage.value().string());
        });

    if (potentialError.has_value() && !potentialError->has_value()) {
      return tl::unexpected(potentialError->error());
    }

    // TODO should check `CompositeImages` is not a testing class instead of
    // checking exactly `ImageCompositor`
    if constexpr (std::is_same_v<CompositeImages, ImageCompositor>) {
      std::chrono::milliseconds sleepTime =
          std::chrono::milliseconds(transition.duration) / transition.steps;
      sleepTime = std::clamp(sleepTime - timeElapsed,
                             std::chrono::milliseconds(0), sleepTime);
      std::this_thread::sleep_for(sleepTime);
    }
  }

  return {};
}

} // namespace dynamic_paper
