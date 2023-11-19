#include "background_setter.hpp"

#include <format>

#include "command_executor.hpp"
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
  default: {
    logAssert(false,
              "unhandled BackgroundSetMode when converting script to command");
  }
  }

  return std::format("{} -m {} {}", scriptName, modeString, imageName);
}

static std::expected<void, BackgroundError>
runCommandHandleError(const std::string &command) {
  const int result = runCommandStdout(command);

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

  std::visit(
      overloaded{
          [mode, &imageName](const BackgroundSetterMethodScript &method) {
            return runCommandHandleError(
                convertScriptNameToCommand(method.script, mode, imageName))
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

} // namespace dynamic_paper
