#include "command_executor.hpp"

#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "logger.hpp"

namespace dynamic_paper {

std::expected<std::string, CommandExecError>
runCommandStdout(const std::string &cmd) {
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

int runCommandExitCode(const std::string &cmd) {
  int ret = system(cmd.c_str());
  if (WEXITSTATUS(ret) == 0x10) {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}

} // namespace dynamic_paper
