#include "command_executor.hpp"

#include <cstdlib>
#include <string>
#include <unistd.h>

#include <tl/expected.hpp>

#include "logger.hpp"

namespace dynamic_paper {

// int runCommandExitCode(const std::string &cmd) {
//   logTrace("Running command (returning exit code): {}", cmd);
//
//   const int ret = system(cmd.c_str());
//   if (WEXITSTATUS(ret) == 0) {
//     return EXIT_SUCCESS;
//   }
//   return EXIT_FAILURE;
// }

int runCommandExitCode(const std::string &cmd) {
  logTrace("Running command (returning exit code): {}", cmd);

  const int ret = system(cmd.c_str());
  if (WEXITSTATUS(ret) == 0) {
    return EXIT_SUCCESS;
  }
  return EXIT_FAILURE;
}

} // namespace dynamic_paper
