#pragma once

#include <expected>
#include <filesystem>
#include <optional>
#include <vector>

#include "background_set_enums.hpp"
#include "background_setter.hpp"

/** Static Background Sets show a wallpaper once and exit */

namespace dynamic_paper {

class StaticBackgroundData {
public:
  std::filesystem::path dataDirectory;
  BackgroundSetMode mode;
  std::vector<std::string> imageNames;

  StaticBackgroundData(std::filesystem::path dataDirectory,
                       BackgroundSetMode mode,
                       std::vector<std::string> imageNames);

  void show(const Config &config) const;
};
} // namespace dynamic_paper
