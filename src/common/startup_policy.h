#pragma once

#include "invocation.h"

#include <optional>
#include <string>

namespace bendiff {

struct StartupError {
    int exitCode = 3;
    std::string title;
    std::string message;
};

// Implements the v1 exit-code policy:
// - 2 for invalid invocation / path mismatch
// - 3 for runtime startup error (simulated for now)
// - std::nullopt means continue startup (exit code will be 0 on normal exit)
std::optional<StartupError> startup_error_for(const Invocation& invocation, bool forceRuntimeStartupError);

} // namespace bendiff
