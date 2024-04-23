#include "background_setter.hpp"

#include <cstdlib>
#include <filesystem>
#include <format>
#include <string>

#include <tl/expected.hpp>

#include "background_set_enums.hpp"
#include "command_executor.hpp"
#include "golang/go-background.h"
#include "logger.hpp"

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

} // namespace

// ===== Header ===============

// TODO remove expected?
tl::expected<void, BackgroundError>
setBackgroundToImage(const std::filesystem::path &imagePath,
                     const BackgroundSetMode mode) {
  logTrace("Setting background to image ({})", imagePath.string());

  const std::string imageName = imagePath.string();

  callSetBackground(imageName, backgroundSetModeString(mode));
  return {};
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
