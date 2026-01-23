#include "file_compare.h"

#include <array>
#include <cstring>
#include <fstream>
#include <system_error>

namespace fs = std::filesystem;

namespace bendiff::core {

namespace {

bool get_file_size_if_possible(const fs::path& p, std::uintmax_t& outSize)
{
    std::error_code ec;
    const auto s = fs::file_size(p, ec);
    if (ec) {
        return false;
    }
    outSize = s;
    return true;
}

} // namespace

FileCompareResult CompareFilesBytewise(const fs::path& left, const fs::path& right)
{
    std::ifstream a(left, std::ios::binary);
    std::ifstream b(right, std::ios::binary);

    if (!a.is_open() || !b.is_open()) {
        return FileCompareResult::Unreadable;
    }

    std::uintmax_t sizeA = 0;
    std::uintmax_t sizeB = 0;
    const bool gotA = get_file_size_if_possible(left, sizeA);
    const bool gotB = get_file_size_if_possible(right, sizeB);

    if (gotA && gotB && sizeA != sizeB) {
        return FileCompareResult::Different;
    }

    std::array<char, 64 * 1024> bufA{};
    std::array<char, 64 * 1024> bufB{};

    while (true) {
        a.read(bufA.data(), static_cast<std::streamsize>(bufA.size()));
        b.read(bufB.data(), static_cast<std::streamsize>(bufB.size()));

        const std::streamsize readA = a.gcount();
        const std::streamsize readB = b.gcount();

        if (a.bad() || b.bad()) {
            return FileCompareResult::Unreadable;
        }

        if (readA != readB) {
            return FileCompareResult::Different;
        }

        if (readA == 0) {
            // Both reached EOF.
            return FileCompareResult::Same;
        }

        if (std::memcmp(bufA.data(), bufB.data(), static_cast<std::size_t>(readA)) != 0) {
            return FileCompareResult::Different;
        }

        if (a.eof() && b.eof()) {
            return FileCompareResult::Same;
        }
    }
}

} // namespace bendiff::core
