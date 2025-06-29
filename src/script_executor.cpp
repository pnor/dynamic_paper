#include "script_executor.hpp"

#include <iostream>

#include <sstream>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

#include "logger.hpp"

namespace dynamic_paper {

namespace {

template <typename T>
concept HasCStr = requires(T stringLike) {
  { stringLike.c_str() } -> std::same_as<const char *>;
};

inline bool statusIsAbnormal(const int status) {
  const bool exitedNormally = WIFEXITED(status);
  const bool exitCodeIsZero = WEXITSTATUS(status) == 0;
  return !exitedNormally || !exitCodeIsZero;
}

void monitorScript(const pid_t pid) {
  int status = 0;
  pid_t ret = 0;

  while ((ret = waitpid(pid, &status, 0)) == -1) {
    if (errno != EINTR) {
      logError("(thread) Script was interrupted while running! \nerrno msg: {}",
               strerror(errno));
      break;
    }
  }

  if ((ret == 0) || statusIsAbnormal(status)) {
    logError("Script encountered issue when run! status = {}", status);
  }
}

inline void checkScriptStatus(const pid_t pid) {
  // std::thread scriptCheckerThread(monitorScript, pid);
  // scriptCheckerThread.detach();
}

template <HasCStr... Args>
[[noreturn]] void becomeAndRunScript(const std::filesystem::path &scriptPath,
                                     Args &&...args) {
  const std::array<char *const, sizeof...(Args) + 2> argv{
      // NOLINTNEXTLINE
      const_cast<char *const>(scriptPath.c_str()),
      // NOLINTNEXTLINE
      const_cast<char *const>(std::forward<Args>(args).c_str())..., nullptr};

  if (execv(scriptPath.c_str(), argv.begin()) == -1) {
    logError("Error when trying to run script: {}.\nError from errno: {}",
             scriptPath.string(), strerror(errno));
    exit(EXIT_FAILURE);
  }

  __builtin_unreachable();
}

template <HasCStr... Args>
[[nodiscard]] tl::expected<void, ScriptError>
runScript(const std::filesystem::path &scriptPath, Args &&...args) {
  std::ostringstream scriptCommand;
  scriptCommand << scriptPath.c_str() << " ";
  ((scriptCommand << " " << args), ...);
  logTrace("Starting to run script: {}", scriptCommand.str());

  const pid_t pid = fork();

  if (pid == -1) {
    return tl::make_unexpected(ScriptError::ForkError);
  }

  if (pid != 0) {
    checkScriptStatus(pid);
    return {};
  }

  becomeAndRunScript(scriptPath, std::forward<Args>(args)...);
}

} // namespace

[[nodiscard]] tl::expected<void, ScriptError>
runHookScript(const std::filesystem::path &scriptPath,
              const std::filesystem::path &imagePath) {
  runScript(scriptPath, imagePath);
  return {};
}

[[nodiscard]] tl::expected<void, ScriptError>
runBackgroundSetScript(const std::filesystem::path &scriptPath,
                       const std::filesystem::path &imagePath,
                       BackgroundSetMode mode) {
  runScript(scriptPath, imagePath.string(), backgroundSetModeString(mode));
  return {};
}

} // namespace dynamic_paper
