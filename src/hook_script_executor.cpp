#include "hook_script_executor.hpp"

#include <sys/wait.h>
#include <thread>
#include <unistd.h>

#include "logger.hpp"

namespace dynamic_paper {

namespace {

inline bool statusIsAbnormal(const int status) {
  const bool exitedNormally = WIFEXITED(status);
  const bool exitCodeIsZero = WEXITSTATUS(status) == 0;
  return !exitedNormally || !exitCodeIsZero;
}

void monitorHookScript(const pid_t pid) {
  int status = 0;
  pid_t ret = 0;

  while ((ret = waitpid(pid, &status, 0)) == -1) {
    if (errno != EINTR) {
      logError(
          "(thread) Hook Script was interrupted while running! \nerrno msg: {}",
          strerror(errno));
      break;
    }
  }

  if ((ret == 0) || statusIsAbnormal(status)) {
    logError("Hook Script encountered issue when run! status = {}", status);
  }
}

inline void checkHookScriptStatus(const pid_t pid) {
  std::thread scriptCheckerThread(monitorHookScript, pid);
  scriptCheckerThread.detach();
}

[[noreturn]] void
becomeAndRunHookScript(const std::filesystem::path &hookScriptPath,
                       const std::filesystem::path &imagePath) {
  // NOLINTNEXTLINE: Need to call execl to execute the hook script
  if (execl(hookScriptPath.c_str(), hookScriptPath.filename().c_str(),
            imagePath.c_str(), nullptr) == -1) {
    logError(
        "Error when trying to run hook script: {} {}.\nError from errno: {}",
        hookScriptPath.string(), imagePath.string(), strerror(errno));
    exit(EXIT_FAILURE);
  }

  // unreachable
  __builtin_unreachable();
}

} // namespace

[[nodiscard]] tl::expected<void, HookError>
runHookScript(const std::filesystem::path &hookScriptPath,
              const std::filesystem::path &imagePath) {
  logTrace("Starting to run hook script: {} {}", hookScriptPath.string(),
           imagePath.string());

  const pid_t pid = fork();

  if (pid == -1) {
    return tl::make_unexpected(HookError::ForkError);
  }

  if (pid != 0) {
    checkHookScriptStatus(pid);
    return {};
  }

  becomeAndRunHookScript(hookScriptPath, imagePath);
}

} // namespace dynamic_paper
