#include "logging.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>

namespace fs = std::filesystem;

namespace bendiff::logging {
namespace {

struct State {
    bool initialized = false;
    Options options{};
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

    fs::path logDir;
    fs::path logFile;
    std::ofstream file;
    std::mutex mutex;
};

State& state()
{
    static State s;
    return s;
}

const char* level_to_string(Level level)
{
    switch (level) {
    case Level::Debug:
        return "DEBUG";
    case Level::Info:
        return "INFO";
    case Level::Warn:
        return "WARN";
    case Level::Error:
        return "ERROR";
    }
    return "INFO";
}

std::string timestamp_now()
{
    using clock = std::chrono::system_clock;
    const auto now = clock::now();
    const auto nowTimeT = clock::to_time_t(now);

    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &nowTimeT);
#else
    localtime_r(&nowTimeT, &tm);
#endif

    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '.' << std::setw(3) << std::setfill('0')
        << ms.count();
    return out.str();
}

std::optional<fs::path> env_path(const char* name)
{
    if (const char* value = std::getenv(name)) {
        if (*value != '\0') {
            return fs::path(value);
        }
    }
    return std::nullopt;
}

fs::path default_log_dir(const std::string& appName)
{
#if defined(_WIN32)
    if (auto base = env_path("LOCALAPPDATA")) {
        return *base / "BenDiff" / "logs";
    }
    return fs::temp_directory_path() / "BenDiff" / "logs";
#else
    if (auto base = env_path("XDG_STATE_HOME")) {
        return *base / appName / "logs";
    }
    if (auto home = env_path("HOME")) {
        return *home / ".local" / "state" / appName / "logs";
    }
    return fs::temp_directory_path() / appName / "logs";
#endif
}

void rotate_if_needed_locked(State& s)
{
    if (s.options.rotateBytes == 0 || s.options.rotateFiles <= 0) {
        return;
    }

    std::error_code ec;
    if (!fs::exists(s.logFile, ec) || ec) {
        return;
    }

    const auto size = fs::file_size(s.logFile, ec);
    if (ec || size < s.options.rotateBytes) {
        return;
    }

    // Close before renaming on Windows.
    if (s.file.is_open()) {
        s.file.flush();
        s.file.close();
    }

    const int maxN = s.options.rotateFiles;
    for (int i = maxN; i >= 1; --i) {
        fs::path from = (i == 1) ? s.logFile : fs::path(s.logFile.string() + "." + std::to_string(i - 1));
        fs::path to = fs::path(s.logFile.string() + "." + std::to_string(i));

        if (fs::exists(from, ec) && !ec) {
            fs::remove(to, ec);
            ec.clear();
            fs::rename(from, to, ec);
            ec.clear();
        } else {
            ec.clear();
        }
    }

    s.file.open(s.logFile, std::ios::out | std::ios::app);
}

void ensure_file_open_locked(State& s)
{
    if (s.file.is_open()) {
        return;
    }

    std::error_code ec;
    fs::create_directories(s.logDir, ec);

    s.file.open(s.logFile, std::ios::out | std::ios::app);
}

} // namespace

void init(const Options& options)
{
    State& s = state();
    std::scoped_lock lock(s.mutex);

    s.options = options;
    s.minimumLevel = options.minimumLevel;
    s.consoleEnabled = options.consoleEnabled;

    s.logDir = default_log_dir(options.appName);
    s.logFile = s.logDir / (options.appName + ".log");

    ensure_file_open_locked(s);
    rotate_if_needed_locked(s);

    s.initialized = true;
}

void shutdown()
{
    State& s = state();
    std::scoped_lock lock(s.mutex);

    if (s.file.is_open()) {
        s.file.flush();
        s.file.close();
    }
    s.initialized = false;
}

void set_minimum_level(Level level)
{
    State& s = state();
    std::scoped_lock lock(s.mutex);
    s.minimumLevel = level;
}

std::string log_directory()
{
    State& s = state();
    std::scoped_lock lock(s.mutex);
    return s.logDir.empty() ? std::string{} : s.logDir.string();
}

void log(Level level, std::string_view message)
{
    State& s = state();
    std::scoped_lock lock(s.mutex);

    if (!s.initialized) {
        init({});
    }

    if (static_cast<int>(level) < static_cast<int>(s.minimumLevel)) {
        return;
    }

    ensure_file_open_locked(s);

    const std::string line = timestamp_now() + " [" + level_to_string(level) + "] " + std::string(message) + "\n";

    if (s.consoleEnabled) {
        std::cerr << line;
        std::cerr.flush();
    }

    if (s.file.is_open()) {
        s.file << line;
        s.file.flush();
        rotate_if_needed_locked(s);
    }
}

} // namespace bendiff::logging
