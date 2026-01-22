#include "process.h"

#include <cerrno>
#include <cstring>
#include <system_error>

#if defined(_WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace bendiff::core {
namespace {

#if defined(_WIN32)

static std::string quote_arg_windows(const std::string& arg)
{
    // Minimal quoting suitable for CreateProcess command line.
    // This is not a full cmd.exe parser; CreateProcess uses its own rules.
    if (arg.empty()) {
        return "\"\"";
    }
    const bool needsQuotes = (arg.find_first_of(" \t\n\v\"") != std::string::npos);
    if (!needsQuotes) {
        return arg;
    }

    std::string out;
    out.push_back('"');
    for (char c : arg) {
        if (c == '"') {
            out += "\\\"";
        } else {
            out.push_back(c);
        }
    }
    out.push_back('"');
    return out;
}

static std::string build_command_line_windows(const std::vector<std::string>& argv)
{
    std::string cmd;
    bool first = true;
    for (const auto& a : argv) {
        if (!first) {
            cmd.push_back(' ');
        }
        first = false;
        cmd += quote_arg_windows(a);
    }
    return cmd;
}

static bool read_all_from_handle(HANDLE h, std::string& out)
{
    char buf[4096];
    DWORD readBytes = 0;
    while (true) {
        const BOOL ok = ReadFile(h, buf, static_cast<DWORD>(sizeof(buf)), &readBytes, nullptr);
        if (!ok || readBytes == 0) {
            break;
        }
        out.append(buf, buf + readBytes);
    }
    return true;
}

#else

static void append_errno(std::string& s, const char* prefix)
{
    s += prefix;
    s += ": ";
    s += std::strerror(errno);
}

static bool set_nonblocking(int fd)
{
    const int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

#endif

} // namespace

ProcessResult RunProcess(const std::vector<std::string>& argv, fs::path workingDir)
{
    ProcessResult result;

    if (argv.empty() || argv[0].empty()) {
        result.exitCode = 127;
        result.stderrText = "RunProcess: empty argv";
        return result;
    }

    std::error_code ec;
    if (!workingDir.empty()) {
        if (!fs::exists(workingDir, ec) || ec || !fs::is_directory(workingDir, ec) || ec) {
            result.exitCode = 127;
            result.stderrText = "RunProcess: workingDir does not exist or is not a directory";
            return result;
        }
    }

#if defined(_WIN32)
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle = TRUE;

    HANDLE outRead = nullptr;
    HANDLE outWrite = nullptr;
    HANDLE errRead = nullptr;
    HANDLE errWrite = nullptr;

    if (!CreatePipe(&outRead, &outWrite, &sa, 0)) {
        result.exitCode = 127;
        result.stderrText = "RunProcess: CreatePipe(stdout) failed";
        return result;
    }
    if (!CreatePipe(&errRead, &errWrite, &sa, 0)) {
        CloseHandle(outRead);
        CloseHandle(outWrite);
        result.exitCode = 127;
        result.stderrText = "RunProcess: CreatePipe(stderr) failed";
        return result;
    }

    // Prevent child from inheriting read ends.
    SetHandleInformation(outRead, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(errRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = outWrite;
    si.hStdError = errWrite;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    std::string cmdLine = build_command_line_windows(argv);
    // CreateProcess may modify the command line buffer.
    std::vector<char> cmdBuf(cmdLine.begin(), cmdLine.end());
    cmdBuf.push_back('\0');

    const std::string wd = workingDir.empty() ? std::string() : workingDir.string();
    const BOOL ok = CreateProcessA(
        nullptr,
        cmdBuf.data(),
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        wd.empty() ? nullptr : wd.c_str(),
        &si,
        &pi);

    // Parent no longer needs write ends.
    CloseHandle(outWrite);
    CloseHandle(errWrite);

    if (!ok) {
        const DWORD gle = GetLastError();
        result.exitCode = 127;
        result.stderrText = "RunProcess: CreateProcess failed (GetLastError=" + std::to_string(static_cast<unsigned long long>(gle)) + ")";
        CloseHandle(outRead);
        CloseHandle(errRead);
        return result;
    }

    // Read output after process exits (simple + sufficient for small outputs like git status/--version).
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    result.exitCode = static_cast<int>(exitCode);

    read_all_from_handle(outRead, result.stdoutText);
    read_all_from_handle(errRead, result.stderrText);

    CloseHandle(outRead);
    CloseHandle(errRead);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    return result;
#else
    int outPipe[2] = {-1, -1};
    int errPipe[2] = {-1, -1};

    if (pipe(outPipe) != 0) {
        result.exitCode = 127;
        append_errno(result.stderrText, "RunProcess: pipe(stdout) failed");
        return result;
    }
    if (pipe(errPipe) != 0) {
        close(outPipe[0]);
        close(outPipe[1]);
        result.exitCode = 127;
        append_errno(result.stderrText, "RunProcess: pipe(stderr) failed");
        return result;
    }

    const pid_t pid = fork();
    if (pid < 0) {
        close(outPipe[0]);
        close(outPipe[1]);
        close(errPipe[0]);
        close(errPipe[1]);
        result.exitCode = 127;
        append_errno(result.stderrText, "RunProcess: fork failed");
        return result;
    }

    if (pid == 0) {
        // Child.
        (void)dup2(outPipe[1], STDOUT_FILENO);
        (void)dup2(errPipe[1], STDERR_FILENO);

        close(outPipe[0]);
        close(outPipe[1]);
        close(errPipe[0]);
        close(errPipe[1]);

        if (!workingDir.empty()) {
            if (chdir(workingDir.c_str()) != 0) {
                const char* msg = "RunProcess: chdir failed\n";
                (void)write(STDERR_FILENO, msg, std::strlen(msg));
                _exit(127);
            }
        }

        std::vector<char*> args;
        args.reserve(argv.size() + 1);
        for (const auto& a : argv) {
            args.push_back(const_cast<char*>(a.c_str()));
        }
        args.push_back(nullptr);

        execvp(args[0], args.data());

        // exec failed.
        const char* prefix = "RunProcess: execvp failed: ";
        (void)write(STDERR_FILENO, prefix, std::strlen(prefix));
        const char* err = std::strerror(errno);
        (void)write(STDERR_FILENO, err, std::strlen(err));
        (void)write(STDERR_FILENO, "\n", 1);
        _exit(127);
    }

    // Parent.
    close(outPipe[1]);
    close(errPipe[1]);

    (void)set_nonblocking(outPipe[0]);
    (void)set_nonblocking(errPipe[0]);

    bool outOpen = true;
    bool errOpen = true;

    while (outOpen || errOpen) {
        struct pollfd fds[2];
        nfds_t nfds = 0;

        if (outOpen) {
            fds[nfds].fd = outPipe[0];
            fds[nfds].events = POLLIN;
            fds[nfds].revents = 0;
            ++nfds;
        }
        if (errOpen) {
            fds[nfds].fd = errPipe[0];
            fds[nfds].events = POLLIN;
            fds[nfds].revents = 0;
            ++nfds;
        }

        const int pr = poll(fds, nfds, 250);
        if (pr < 0) {
            if (errno == EINTR) {
                continue;
            }
            // Bail out, but still wait for the child.
            append_errno(result.stderrText, "RunProcess: poll failed");
            break;
        }

        auto drain = [](int fd, std::string& dst, bool& openFlag) {
            char buf[4096];
            while (true) {
                const ssize_t n = read(fd, buf, sizeof(buf));
                if (n > 0) {
                    dst.append(buf, buf + n);
                    continue;
                }
                if (n == 0) {
                    openFlag = false;
                    break;
                }
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }
                if (errno == EINTR) {
                    continue;
                }
                openFlag = false;
                break;
            }
        };

        if (outOpen) {
            drain(outPipe[0], result.stdoutText, outOpen);
        }
        if (errOpen) {
            drain(errPipe[0], result.stderrText, errOpen);
        }
    }

    close(outPipe[0]);
    close(errPipe[0]);

    int status = 0;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno == EINTR) {
            continue;
        }
        append_errno(result.stderrText, "RunProcess: waitpid failed");
        result.exitCode = 127;
        return result;
    }

    if (WIFEXITED(status)) {
        result.exitCode = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        result.exitCode = 128 + WTERMSIG(status);
    } else {
        result.exitCode = 127;
    }

    return result;
#endif
}

} // namespace bendiff::core
