#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace bendiff::core {

struct ProcessResult {
    int exitCode = 0;
    std::string stdoutText;
    std::string stderrText;
};

// Runs argv[0] with arguments argv[1..] in workingDir.
//
// Notes:
// - This is Qt-free by design (core logic portability).
// - If the process cannot be executed, exitCode will be 127 and stderrText will contain a message.
ProcessResult RunProcess(const std::vector<std::string>& argv, std::filesystem::path workingDir);

} // namespace bendiff::core
