#include "background_setter.hpp"

#include <cstdlib>
#include <filesystem>

#include "format.hpp"

namespace dynamic_paper {

// ===== Header ===============

void setBackgroundToImage(const std::filesystem::path &imagePath,
                          const BackgroundSetMode mode) {
  logTrace("Setting background to image ({})", imagePath.string());

  const std::string imageName = imagePath.string();

  constexpr std::string_view commandTemplate =
      R"(osascript -e '
      tell application "System Events"
      tell every desktop
          set picture to "{}"
      end tell
      end tell')";

  const std::string command = dynamic_paper::format(commandTemplate, imageName);

  system(command.c_str()); // NOLINT
}

} // namespace dynamic_paper
