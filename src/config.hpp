#pragma once

#include <filesystem>
#include <optional>

// General Configuration

enum class BackgroundSetterMethod { Script, WallUtils };

namespace dynamic_paper {
class Config {
public:
  std::filesystem::path backgroundImageDir;
  BackgroundSetterMethod backgroundSetterMethod;
  std::optional<std::filesystem::path> hookScript;

  Config(std::filesystem::path imageDir, BackgroundSetterMethod method,
         std::optional<std::filesystem::path> hookScript);

private:
};

Config loadConfigFromFile(std::filesystem::path path);

} // namespace dynamic_paper
