#include "background_setter.hpp"

#include <algorithm>
#include <expected>
#include <format>
#include <thread>

#include "command_executor.hpp"
#include "image_compositor.hpp"
#include "logger.hpp"
#include "variant_visitor_templ.hpp"

namespace dynamic_paper {

// Static Commmands Run in Shell
constexpr std::string_view WALLUTILS_SET_CENTERED_FORMAT_STRING =
    "setwallpaper -m center {}";
constexpr std::string_view WALLUTILS_SET_FILLED_FORMAT_STRING =
    "setwallpaper -m fill {}";

static inline std::string
convertScriptNameToCommand(const std::string &scriptName,
                           const BackgroundSetMode mode,
                           const std::string &imageName) {
  std::string modeString;
  switch (mode) {
  case BackgroundSetMode::Center: {
    modeString = "center";
    break;
  }
  case BackgroundSetMode::Fill: {
    modeString = "fill";
    break;
  }
  }

  return std::format("{} -m {} {}", scriptName, modeString, imageName);
}

static std::expected<void, BackgroundError>
runCommandHandleError(const std::string &command) {
  const int result = runCommandExitCode(command);

  if (result != 0) {
    logError("Command ''" + command + "' did not run succesfully");
    return std::unexpected(BackgroundError::CommandError);
  }
}

std::expected<void, BackgroundError>
setBackgroundToImage(const std::filesystem::path &imagePath,
                     const BackgroundSetMode mode,
                     const BackgroundSetterMethod &method) {
  const std::string imageName = imagePath.string();

  return std::visit(
      overloaded{
          [mode, &imageName](const BackgroundSetterMethodScript &method) {
            return runCommandHandleError(
                convertScriptNameToCommand(method.script, mode, imageName));
          },
          [mode, &imageName](const BackgroundSetterMethodWallUtils) {
            switch (mode) {
            case BackgroundSetMode::Center: {
              const std::string command =
                  std::format(WALLUTILS_SET_CENTERED_FORMAT_STRING, imageName);

              return runCommandHandleError(command);
            }
            case BackgroundSetMode::Fill: {
              const std::string command =
                  std::format(WALLUTILS_SET_FILLED_FORMAT_STRING, imageName);

              return runCommandHandleError(command);
            }
            }
          }},
      method);
}

std::expected<void, BackgroundError> lerpBackgroundBetweenImages(
    const std::filesystem::path &commonImageDirectory,
    const std::string &beforeImageName, const std::string &afterImageName,
    const std::filesystem::path &cacheDirectory, const unsigned int duration,
    const unsigned int numSteps, const BackgroundSetMode mode,
    const BackgroundSetterMethod &method) {

  for (unsigned int i = 0; i < numSteps; i++) {
    const float percentageFloat = i / static_cast<float>(numSteps);
    const unsigned int percentage = std::clamp(
        static_cast<unsigned int>(percentageFloat * 100.0), 0u, 100u);

    std::expected<std::filesystem::path, CompositeImageError>
        expectedCompositedImage =
            getCompositedImage(commonImageDirectory, beforeImageName,
                               afterImageName, cacheDirectory, percentage);

    if (!expectedCompositedImage.has_value()) {
      return std::unexpected(BackgroundError::CompositeImageError);
    }

    std::expected<void, BackgroundError> backgroundResult =
        setBackgroundToImage(expectedCompositedImage.value(), mode, method);

    if (!backgroundResult.has_value()) {
      return std::unexpected(BackgroundError::SetBackgroundError);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(
        static_cast<unsigned int>(1000 * (1.0f / numSteps) * duration)));
  }
}

std::expected<void, HookCommandError>
runHookCommand(const std::filesystem::path hookScriptPath,
               const std::filesystem::path imagePath) {
  const std::string commandStr =
      std::format("{} {}", hookScriptPath.string(), imagePath.string());
  const int result = runCommandExitCode(commandStr);

  if (result != EXIT_SUCCESS) {
    return std::unexpected(HookCommandError::CommandError);
  }
}
} // namespace dynamic_paper
