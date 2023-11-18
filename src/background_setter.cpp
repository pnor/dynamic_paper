#include "background_setter.hpp"
#include "command_executor.hpp"

namespace dynamic_paper {

std::expected<bool, BackgroundError>
setBackgroundToImage(const std::filesystem::path &imagePath,
                     const BackgroundSetMode mode, const Config &config) {
  //  TODO visit
  // switch (config.backgroundSetterMethod) {
  // case BackgroundSetterMethod::Script: {
  //   if (!config.hookScript.has_value()) {
  //     return std::unexpected(BackgroundError::NoHookScriptFound);
  //   }
  // }
  // case BackgroundSetterMethod::WallUtils: {
  //   // TODO
  //   // switch () {}
  //   std::expected<std::string, CommandExecError> result = runCommand("");
  //   break;
  // }
  // }
  return std::unexpected(BackgroundError::CommandError);
}

} // namespace dynamic_paper
