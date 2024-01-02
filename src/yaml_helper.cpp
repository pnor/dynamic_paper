#include "yaml_helper.hpp"

namespace dynamic_paper {

std::expected<BackgroundSetterMethod, BackgroundSetterMethodError>
parseBackgroundSetterMethod(YAML::Node config, const std::string_view methodKey,
                            const std::string_view scriptKey) {
  YAML::Node node = config[methodKey];

  if (!node.IsDefined()) {
    return std::unexpected(BackgroundSetterMethodError::NoMethodInYAML);
  }

  const std::string &s = node.as<std::string>();
  if (s == SCRIPT_STRING) {
    const YAML::Node scriptNode = config[scriptKey];

    if (!scriptNode.IsDefined()) {
      logError("Method was 'script' but no 'script: ' yaml field with name "
               "'method_script' was provided");
      return std::unexpected(BackgroundSetterMethodError::NoScriptProvided);
    }

    std::optional<std::filesystem::path> methodScriptPath =
        stringTo<std::filesystem::path>(scriptNode.as<std::string>());

    if (!methodScriptPath.has_value()) {
      logError("Method was 'script' but no script was able to be parsed");
      return std::unexpected(BackgroundSetterMethodError::NoScriptProvided);
    }

    return BackgroundSetterMethodScript(methodScriptPath.value());
  } else if (s == WALLUTILS_STRING) {
    return BackgroundSetterMethodWallUtils();
  } else {
    logError("String " + s + " doesn't correspond to valid method");
    return std::unexpected(BackgroundSetterMethodError::InvalidMethod);
  }
}

} // namespace dynamic_paper
