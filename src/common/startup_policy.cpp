#include "startup_policy.h"

namespace bendiff {

std::optional<StartupError> startup_error_for(const Invocation& invocation, bool forceRuntimeStartupError)
{
    if (forceRuntimeStartupError) {
        StartupError err;
        err.exitCode = 3;
        err.title = "Startup error";
        err.message = "A runtime startup error was forced (simulated).";
        return err;
    }

    if (invocation.mode == AppMode::Invalid) {
        StartupError err;
        err.exitCode = 2;
        err.title = "Invalid invocation";
        err.message = invocation.error.empty() ? "Invalid arguments." : invocation.error;
        return err;
    }

    return std::nullopt;
}

} // namespace bendiff
