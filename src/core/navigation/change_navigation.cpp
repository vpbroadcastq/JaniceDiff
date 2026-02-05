#include "change_navigation.h"

namespace bendiff::core::navigation {

std::vector<ChangeLocation> EnumerateChangeHunks(const diff::DiffResult& d)
{
    std::vector<ChangeLocation> out;
    out.reserve(d.hunks.size());

    for (std::size_t i = 0; i < d.hunks.size(); ++i) {
        const auto& h = d.hunks[i];
        out.push_back(ChangeLocation{
            .hunkIndex = i,
            .leftStart = h.leftStart,
            .leftCount = h.leftCount,
            .rightStart = h.rightStart,
            .rightCount = h.rightCount,
        });
    }

    return out;
}

std::optional<std::size_t> NextChangeIndex(std::optional<std::size_t> currentIndex,
                                          std::span<const ChangeLocation> changes)
{
    if (changes.empty()) {
        return std::nullopt;
    }

    if (!currentIndex.has_value() || *currentIndex >= changes.size()) {
        return 0;
    }

    const std::size_t next = *currentIndex + 1;
    if (next >= changes.size()) {
        return std::nullopt;
    }

    return next;
}

std::optional<std::size_t> PrevChangeIndex(std::optional<std::size_t> currentIndex,
                                          std::span<const ChangeLocation> changes)
{
    if (changes.empty()) {
        return std::nullopt;
    }

    if (!currentIndex.has_value() || *currentIndex >= changes.size()) {
        return changes.size() - 1;
    }

    if (*currentIndex == 0) {
        return std::nullopt;
    }

    return *currentIndex - 1;
}

} // namespace bendiff::core::navigation
