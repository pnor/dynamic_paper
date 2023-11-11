#include "command_executor.hpp"

#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "logger.hpp"

namespace dynamic_paper {

std::expected<std::string, CommandExecError>
runCommand(const std::string &cmd) {
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"),
                                                pclose);
  if (!pipe) {
    logError("Unable to open pipe when running the command: " + cmd);
    return std::unexpected(CommandExecError::PopenFail);
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}
} // namespace dynamic_paper
