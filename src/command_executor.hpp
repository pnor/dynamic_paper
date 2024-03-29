#pragma once

/**
 * Execute shell commands and return the output
 */

#include <string>

#include <tl/expected.hpp>

namespace dynamic_paper {

enum class CommandExecError { PopenFail };

/** Runs `cmd` and returns the output in a string */
tl::expected<std::string, CommandExecError>
runCommandStdout(const std::string &cmd);

/** Runs `cmd` and returns the exit code */
int runCommandExitCode(const std::string &cmd);

} // namespace dynamic_paper
