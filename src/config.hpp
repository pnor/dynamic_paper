#pragma once

#include <filesystem>
#include <optional>

// General Configuration

namespace dynamic_paper {

enum class BackgroundSetterMethod { Script, WallUtils };

struct ConfigDefaults {
  static constexpr std::string_view backgroundImageDir =
      "~/.local/share/dynamic_paper/images";
  static constexpr BackgroundSetterMethod backgroundSetterMethod =
      BackgroundSetterMethod::WallUtils;

  ConfigDefaults() = delete;
  ConfigDefaults(ConfigDefaults &other) = delete;
  ConfigDefaults(ConfigDefaults &&other) = delete;
  ~ConfigDefaults() = delete;
};

class Config {
public:
  std::filesystem::path backgroundImageDir;
  BackgroundSetterMethod backgroundSetterMethod;
  std::optional<std::filesystem::path> hookScript;

  Config(std::filesystem::path imageDir, BackgroundSetterMethod method,
         std::optional<std::filesystem::path> hookScript);
};

Config loadConfigFromFile(std::filesystem::path path);

} // namespace dynamic_paper
