#include "command_executor.hpp"

#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "logger.hpp"

namespace dynamic_paper {

tl::expected<std::string, CommandExecError>
runCommandStdout(const std::string &cmd) {
  logTrace("Running command (returning stdout): {}", cmd);
  constexpr size_t BUFFER_SIZE = 128;
  std::array<char, BUFFER_SIZE> buffer{};
  std::string result;
  const std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"),
                                                      pclose);
  if (!pipe) {
    logError("Unable to open pipe when running the command: {}", cmd);
    return tl::unexpected(CommandExecError::PopenFail);
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

int runCommandExitCode(const std::string &cmd) {
  logTrace("Running command (returning exit code): {}", cmd);

  const int ret = system(cmd.c_str());
  if (WEXITSTATUS(ret) == 0) {
    return EXIT_SUCCESS;
  }
  return EXIT_FAILURE;
}

} // namespace dynamic_paper
