#include <filesystem>
#include <string>

#include <tl/expected.hpp>

#include "background_set_enums.hpp"
#include "golang/go-background.h"
#include "logger.hpp"
#include "script_executor.hpp"
#include "background_setter.hpp"

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

void setBackgroundToImage(const std::filesystem::path &imagePath,
                          const BackgroundSetMode mode) {
  logTrace("Setting background to image ({})", imagePath.string());

  const std::string imageName = imagePath.string();

  callSetBackground(imageName, backgroundSetModeString(mode));
}

void setBackgroundToImageUsingScript(const std::filesystem::path &scriptPath,
                                     const std::filesystem::path &imagePath,
                                     BackgroundSetMode mode) {
  logTrace("Setting background to image ({})", imagePath.string());

  const tl::expected<void, ScriptError> scriptResult =
      runBackgroundSetScript(scriptPath, imagePath, mode);

  if (!scriptResult.has_value()) {
    logError("Error relating to forking occured when running background "
             "setting script");
  }
}

} // namespace dynamic_paper
