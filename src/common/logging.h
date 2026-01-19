#pragma once

#include <string>
#include <string_view>

namespace bendiff::logging {

enum class Level {
    Debug,
    Info,
    Warn,
    Error,
};

struct Options {
    Level minimumLevel =
#ifdef NDEBUG
        Level::Info;
#else
        Level::Debug;
#endif

    bool consoleEnabled =
#ifdef NDEBUG
        false;
#else
        true;
#endif

    std::size_t rotateBytes = 1024 * 1024; // 1 MiB
    int rotateFiles = 3;                  // keep bendiff.log, .1, .2, .3
    std::string appName = "bendiff";
};

// Initializes logging sinks (console + rotating file). Safe to call multiple times.
void init(const Options& options = {});

// Flushes and closes any open log file.
void shutdown();

void set_minimum_level(Level level);

// Logs a single line. A newline is appended automatically.
void log(Level level, std::string_view message);

inline void debug(std::string_view message) { log(Level::Debug, message); }
inline void info(std::string_view message) { log(Level::Info, message); }
inline void warn(std::string_view message) { log(Level::Warn, message); }
inline void error(std::string_view message) { log(Level::Error, message); }

// Returns the directory used for logs (created on init). Empty if not initialized yet.
std::string log_directory();

} // namespace bendiff::logging
