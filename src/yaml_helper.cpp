#include "yaml_helper.hpp"

namespace dynamic_paper {

tl::expected<BackgroundSetterMethod, BackgroundSetterMethodError>
parseBackgroundSetterMethod(YAML::Node config, const std::string_view methodKey,
                            const std::string_view scriptKey) {
  const YAML::Node node = config[methodKey];

  if (!node.IsDefined()) {
    return tl::unexpected(BackgroundSetterMethodError::NoMethodInYAML);
  }

  const std::string &nodeString = node.as<std::string>();

  if (nodeString == SCRIPT_STRING) {
    const YAML::Node scriptNode = config[scriptKey];

    if (!scriptNode.IsDefined()) {
      logError("Method was 'script' but no 'script: ' yaml field with name "
               "'method_script' was provided");
      return tl::unexpected(BackgroundSetterMethodError::NoScriptProvided);
    }

    std::optional<std::filesystem::path> methodScriptPath =
        yamlStringTo<std::filesystem::path>(scriptNode.as<std::string>());

    if (!methodScriptPath.has_value()) {
      logError("Method was 'script' but no script was able to be parsed");
      return tl::unexpected(BackgroundSetterMethodError::NoScriptProvided);
    }

    return BackgroundSetterMethodScript(methodScriptPath.value());
  }

  if (nodeString == WALLUTILS_STRING) {
    return BackgroundSetterMethodWallUtils();
  }

  logError("String '{}' doesn't correspond to valid method", nodeString);
  return tl::unexpected(BackgroundSetterMethodError::InvalidMethod);
}

} // namespace dynamic_paper
