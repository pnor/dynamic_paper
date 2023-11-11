#pragma once

#include <expected>
#include <string>

namespace dynamic_paper {

enum class CommandExecError { PopenFail };

/** Runs `cmd` and returns the output in a string */
std::expected<std::string, CommandExecError> runCommand(const std::string &cmd);

} // namespace dynamic_paper
