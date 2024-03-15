#include "background_setter.hpp"

#include <algorithm>
#include <format>
#include <thread>

#include <tl/expected.hpp>

#include "command_executor.hpp"
#include "file_util.hpp"
#include "image_compositor.hpp"
#include "logger.hpp"
#include "variant_visitor_templ.hpp"

// TODO move
#include "golang/go-background.h"

namespace dynamic_paper {

namespace {

// Static Commmands Run in Shell
constexpr std::string_view WALLUTILS_SET_CENTERED_FORMAT_STRING =
    "setwallpaper -m center {}";
constexpr std::string_view WALLUTILS_SET_FILLED_FORMAT_STRING =
    "setwallpaper -m fill {}";

inline std::string convertScriptNameToCommand(const std::string &scriptName,
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

tl::expected<void, BackgroundError>
runCommandHandleError(const std::string &command) {
  const int result = runCommandExitCode(command);

  if (result != 0) {
    logError("Command '{}' did not run succesfully", command);
    return tl::unexpected(BackgroundError::CommandError);
  }

  return {};
}

} // namespace

// ===== Header ===============

tl::expected<void, BackgroundError>
setBackgroundToImage(const std::filesystem::path &imagePath,
                     const BackgroundSetMode mode,
                     const BackgroundSetterMethod &method) {
  logTrace("Setting background to image ({})", imagePath.string());

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

            logAssert(false, "Unhandled background set mode case when setting "
                             "background to image!");
            return tl::expected<void, BackgroundError>();
          }},
      method);
}

tl::expected<void, HookCommandError>
runHookCommand(const std::filesystem::path &hookScriptPath,
               const std::filesystem::path &imagePath) {
  logTrace("Running hook command: ({})", hookScriptPath.string());

  const std::string commandStr =
      std::format("{} {}", hookScriptPath.string(), imagePath.string());
  const int result = runCommandExitCode(commandStr);

  if (result != EXIT_SUCCESS) {
    return tl::unexpected(HookCommandError::CommandError);
  }

  return {};
}
} // namespace dynamic_paper
