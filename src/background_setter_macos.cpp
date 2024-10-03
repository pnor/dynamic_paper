#include "background_setter.hpp"

#include <cstdlib>
#include <filesystem>
#include <format>

namespace dynamic_paper {

// ===== Header ===============

void setBackgroundToImage(const std::filesystem::path &imagePath,
                          const BackgroundSetMode mode) {
  logTrace("Setting background to image ({})", imagePath.string());

  const std::string imageName = imagePath.string();

  constexpr std::string_view commandTemplate =
      R"(osascript -e 'tell app "Finder" to open POSIX file "{}"')";

  const std::string command = std::format(commandTemplate, imageName);

  system(command.c_str());
}

} // namespace dynamic_paper
