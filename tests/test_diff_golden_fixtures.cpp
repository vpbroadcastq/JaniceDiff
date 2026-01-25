#include <diff/alignment.h>
#include <diff/diff.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace bendiff::core::diff {
namespace {

fs::path fixture_root()
{
    // tests/test_diff_golden_fixtures.cpp -> tests/fixtures/diff
    const fs::path here = fs::path(__FILE__).parent_path();
    return here / "fixtures" / "diff";
}

std::string SlurpFile(const fs::path& p)
{
    std::ifstream in(p, std::ios::binary);
    EXPECT_TRUE(in.good()) << p;

    std::ostringstream out;
    out << in.rdbuf();
    return out.str();
}

void WriteFile(const fs::path& p, const std::string& bytes)
{
    fs::create_directories(p.parent_path());
    std::ofstream out(p, std::ios::binary);
    ASSERT_TRUE(out.good()) << p;
    out << bytes;
}

std::vector<std::string> ReadLinesNormalized(const fs::path& p)
{
    std::ifstream in(p, std::ios::binary);
    EXPECT_TRUE(in.good()) << p;

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    return lines;
}

std::string ToString(WhitespaceMode mode)
{
    switch (mode) {
        case WhitespaceMode::Exact:
            return "Exact";
        case WhitespaceMode::IgnoreTrailing:
            return "IgnoreTrailing";
        case WhitespaceMode::IgnoreAll:
            return "IgnoreAll";
    }
    return "<unknown>";
}

std::string ToString(LineOp op)
{
    switch (op) {
        case LineOp::Equal:
            return "EQ";
        case LineOp::Insert:
            return "INS";
        case LineOp::Delete:
            return "DEL";
    }
    return "<?>";
}

std::string IndexOrDash(std::optional<std::size_t> idx)
{
    if (!idx) {
        return "-";
    }
    return std::to_string(*idx);
}

std::string IndexOrDash(std::size_t idx, std::size_t npos)
{
    if (idx == npos) {
        return "-";
    }
    return std::to_string(idx);
}

std::string SerializeDiffResult(const DiffResult& r)
{
    std::ostringstream out;
    out << "MODE " << ToString(r.mode) << "\n";
    out << "LEFT " << r.leftLineCount << " RIGHT " << r.rightLineCount << "\n";

    out << "HUNKS " << r.hunks.size() << "\n";
    for (std::size_t i = 0; i < r.hunks.size(); ++i) {
        const auto& h = r.hunks[i];
        out << "HUNK " << i << " L " << h.leftStart << " " << h.leftCount << " R " << h.rightStart << " "
            << h.rightCount << "\n";
        for (const auto& dl : h.lines) {
            out << "  " << ToString(dl.op) << " L " << IndexOrDash(dl.leftIndex, DiffLine::npos) << " R "
                << IndexOrDash(dl.rightIndex, DiffLine::npos) << "\n";
        }
    }

    const auto rows = BuildAlignedRows(r);
    out << "ALIGNED " << rows.size() << "\n";
    for (const auto& row : rows) {
        out << "  " << ToString(row.op) << " L " << IndexOrDash(row.left) << " R " << IndexOrDash(row.right)
            << "\n";
    }

    return out.str();
}

bool UpdateGoldensEnabled()
{
    const char* env = std::getenv("BENDIFF_UPDATE_GOLDENS");
    return env != nullptr && std::string(env) == "1";
}

fs::path ExpectedPathFor(const fs::path& fixtureDir, WhitespaceMode mode)
{
    switch (mode) {
        case WhitespaceMode::Exact:
            return fixtureDir / "expected_exact.txt";
        case WhitespaceMode::IgnoreTrailing:
            return fixtureDir / "expected_ignore_trailing.txt";
        case WhitespaceMode::IgnoreAll:
            return fixtureDir / "expected_ignore_all.txt";
    }
    return fixtureDir / "expected_<unknown>.txt";
}

} // namespace

TEST(DiffGoldenFixtures, FixturesMatchExpectedOutput)
{
    const fs::path root = fixture_root();
    ASSERT_TRUE(fs::exists(root)) << root;

    std::vector<fs::path> fixtureDirs;
    for (const auto& entry : fs::directory_iterator(root)) {
        if (entry.is_directory()) {
            fixtureDirs.push_back(entry.path());
        }
    }
    std::sort(fixtureDirs.begin(), fixtureDirs.end());
    ASSERT_FALSE(fixtureDirs.empty()) << "No fixtures found under " << root;

    const bool updateGoldens = UpdateGoldensEnabled();

    for (const auto& fixtureDir : fixtureDirs) {
        SCOPED_TRACE(fixtureDir);

        const auto leftPath = fixtureDir / "left.txt";
        const auto rightPath = fixtureDir / "right.txt";
        ASSERT_TRUE(fs::exists(leftPath)) << leftPath;
        ASSERT_TRUE(fs::exists(rightPath)) << rightPath;

        const auto leftLines = ReadLinesNormalized(leftPath);
        const auto rightLines = ReadLinesNormalized(rightPath);

        const WhitespaceMode modes[] = {
            WhitespaceMode::Exact,
            WhitespaceMode::IgnoreTrailing,
            WhitespaceMode::IgnoreAll,
        };

        for (const auto mode : modes) {
            SCOPED_TRACE(ToString(mode));

            const auto r = DiffLines(leftLines, rightLines, mode);
            const auto actual = SerializeDiffResult(r);

            const auto expectedPath = ExpectedPathFor(fixtureDir, mode);
            if (updateGoldens) {
                WriteFile(expectedPath, actual);
                continue;
            }

            ASSERT_TRUE(fs::exists(expectedPath))
                << "Missing expected output: " << expectedPath
                << " (set BENDIFF_UPDATE_GOLDENS=1 to generate)";

            const auto expected = SlurpFile(expectedPath);
            EXPECT_EQ(actual, expected) << "Golden mismatch for fixture: " << fixtureDir.filename().string();
        }
    }
}

} // namespace bendiff::core::diff
