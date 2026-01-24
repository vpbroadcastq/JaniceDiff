#include <gtest/gtest.h>

#include <loaded_text_file.h>

#include <filesystem>
#include <fstream>
#include <chrono>
#include <string>

namespace bendiff::core {

namespace fs = std::filesystem;

static fs::path make_unique_temp_dir(const std::string& prefix)
{
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    const auto stamp = std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(now).count());

    fs::path dir = fs::temp_directory_path() / (prefix + "_" + stamp);
    fs::remove_all(dir);
    fs::create_directories(dir);
    return dir;
}

static void write_bytes(const fs::path& path, const std::string& bytes)
{
    std::ofstream out(path, std::ios::binary);
    ASSERT_TRUE(out.good());
    out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    ASSERT_TRUE(out.good());
}

TEST(LoadedTextFile, CanConstruct)
{
    LoadedTextFile f;
    f.absolutePath = "/tmp/example.txt";
    f.status = LoadStatus::Ok;
    f.lines = {"a", "b"};
    f.hadFinalNewline = true;

    EXPECT_EQ(f.status, LoadStatus::Ok);
    ASSERT_EQ(f.lines.size(), 2u);
    EXPECT_EQ(f.lines[0], "a");
    EXPECT_EQ(f.lines[1], "b");
    EXPECT_TRUE(f.hadFinalNewline);
}

TEST(SplitLinesNormalizeNewlines, EmptyIsNoLines)
{
    const auto r = SplitLinesNormalizeNewlines("");
    EXPECT_TRUE(r.lines.empty());
    EXPECT_FALSE(r.hadFinalNewline);
}

TEST(SplitLinesNormalizeNewlines, NoNewlineSingleLine)
{
    const auto r = SplitLinesNormalizeNewlines("abc");
    ASSERT_EQ(r.lines.size(), 1u);
    EXPECT_EQ(r.lines[0], "abc");
    EXPECT_FALSE(r.hadFinalNewline);
}

TEST(SplitLinesNormalizeNewlines, LfSplitting)
{
    const auto r = SplitLinesNormalizeNewlines("a\nb\nc");
    ASSERT_EQ(r.lines.size(), 3u);
    EXPECT_EQ(r.lines[0], "a");
    EXPECT_EQ(r.lines[1], "b");
    EXPECT_EQ(r.lines[2], "c");
    EXPECT_FALSE(r.hadFinalNewline);
}

TEST(SplitLinesNormalizeNewlines, FinalLfPreservedFlag)
{
    const auto r = SplitLinesNormalizeNewlines("a\nb\nc\n");
    ASSERT_EQ(r.lines.size(), 3u);
    EXPECT_EQ(r.lines[2], "c");
    EXPECT_TRUE(r.hadFinalNewline);
}

TEST(SplitLinesNormalizeNewlines, LoneLfIsOneEmptyLine)
{
    const auto r = SplitLinesNormalizeNewlines("\n");
    ASSERT_EQ(r.lines.size(), 1u);
    EXPECT_EQ(r.lines[0], "");
    EXPECT_TRUE(r.hadFinalNewline);
}

TEST(SplitLinesNormalizeNewlines, TwoLfsAreTwoEmptyLines)
{
    const auto r = SplitLinesNormalizeNewlines("\n\n");
    ASSERT_EQ(r.lines.size(), 2u);
    EXPECT_EQ(r.lines[0], "");
    EXPECT_EQ(r.lines[1], "");
    EXPECT_TRUE(r.hadFinalNewline);
}

TEST(SplitLinesNormalizeNewlines, CrLfSplitting)
{
    const auto r = SplitLinesNormalizeNewlines("a\r\nb\r\n");
    ASSERT_EQ(r.lines.size(), 2u);
    EXPECT_EQ(r.lines[0], "a");
    EXPECT_EQ(r.lines[1], "b");
    EXPECT_TRUE(r.hadFinalNewline);
}

TEST(SplitLinesNormalizeNewlines, LoneCrSplitting)
{
    const auto r = SplitLinesNormalizeNewlines("a\rb\r");
    ASSERT_EQ(r.lines.size(), 2u);
    EXPECT_EQ(r.lines[0], "a");
    EXPECT_EQ(r.lines[1], "b");
    EXPECT_TRUE(r.hadFinalNewline);
}

TEST(SplitLinesNormalizeNewlines, MixedEndings)
{
    const auto r = SplitLinesNormalizeNewlines("a\r\nb\nc\rd");
    ASSERT_EQ(r.lines.size(), 4u);
    EXPECT_EQ(r.lines[0], "a");
    EXPECT_EQ(r.lines[1], "b");
    EXPECT_EQ(r.lines[2], "c");
    EXPECT_EQ(r.lines[3], "d");
    EXPECT_FALSE(r.hadFinalNewline);
}

TEST(LoadUtf8TextFile, LfOnly)
{
    const fs::path dir = make_unique_temp_dir("bendiff_loadutf8_lf");
    const fs::path file = dir / "lf.txt";
    write_bytes(file, "a\nb\n");

    const auto loaded = LoadUtf8TextFile(file);
    EXPECT_EQ(loaded.status, LoadStatus::Ok);
    EXPECT_EQ(loaded.absolutePath, file);
    EXPECT_EQ(loaded.lines, (std::vector<std::string>{"a", "b"}));
    EXPECT_TRUE(loaded.hadFinalNewline);
}

TEST(LoadUtf8TextFile, CrLf)
{
    const fs::path dir = make_unique_temp_dir("bendiff_loadutf8_crlf");
    const fs::path file = dir / "crlf.txt";
    write_bytes(file, "a\r\nb\r\n");

    const auto loaded = LoadUtf8TextFile(file);
    EXPECT_EQ(loaded.status, LoadStatus::Ok);
    EXPECT_EQ(loaded.lines, (std::vector<std::string>{"a", "b"}));
    EXPECT_TRUE(loaded.hadFinalNewline);
}

TEST(LoadUtf8TextFile, MixedEndings)
{
    const fs::path dir = make_unique_temp_dir("bendiff_loadutf8_mixed");
    const fs::path file = dir / "mixed.txt";
    write_bytes(file, "a\r\nb\nc\rd");

    const auto loaded = LoadUtf8TextFile(file);
    EXPECT_EQ(loaded.status, LoadStatus::Ok);
    EXPECT_EQ(loaded.lines, (std::vector<std::string>{"a", "b", "c", "d"}));
    EXPECT_FALSE(loaded.hadFinalNewline);
}

TEST(LoadUtf8TextFile, InvalidUtf8IsNotUtf8)
{
    const fs::path dir = make_unique_temp_dir("bendiff_loadutf8_invalid");
    const fs::path file = dir / "invalid.bin";

    // Invalid UTF-8: 0xC3 must be followed by a continuation byte.
    const std::string bytes = std::string("x") + std::string(1, static_cast<char>(0xC3)) + std::string("(");
    write_bytes(file, bytes);

    const auto loaded = LoadUtf8TextFile(file);
    EXPECT_EQ(loaded.status, LoadStatus::NotUtf8);
}

TEST(LoadUtf8TextFile, MissingFileIsNotFound)
{
    const fs::path dir = make_unique_temp_dir("bendiff_loadutf8_missing");
    const fs::path file = dir / "does_not_exist.txt";
    const auto loaded = LoadUtf8TextFile(file);
    EXPECT_EQ(loaded.status, LoadStatus::NotFound);
}

} // namespace bendiff::core
