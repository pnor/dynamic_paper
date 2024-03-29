#include "background_setter.hpp"

#include <format>

#include <tl/expected.hpp>

#include "command_executor.hpp"
#include "golang/go-background.h"
#include "logger.hpp"
#include "variant_visitor_templ.hpp"

namespace dynamic_paper {

namespace {

// ===== Calling Set Background in Go ===============

void callSetBackground(const std::string &imageName,
                       const std::string &modeString) {
  /**
   * cgo creates a C compatible header file that does not use `const`.
   * This single function will for a fact not mutate the value referenced by the
   * pointers, despite them being passed as mutable pointers.
   */
  char *imageNamePtr = const_cast<char *>(imageName.c_str());   // NOLINT
  char *modeStringPtr = const_cast<char *>(modeString.c_str()); // NOLINT

  SetBackground(imageNamePtr, modeStringPtr);
}

// ===== Heleper ===============

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
            callSetBackground(imageName, backgroundSetModeString(mode));
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
