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

TEST(LoadUtf8TextFromBytes, ValidUtf8Loads)
{
    const auto loaded = LoadUtf8TextFromBytes("hi\nthere\n", "HEAD:demo.txt");
    EXPECT_EQ(loaded.status, LoadStatus::Ok);
    EXPECT_EQ(loaded.lines, (std::vector<std::string>{"hi", "there"}));
    EXPECT_TRUE(loaded.hadFinalNewline);
    EXPECT_TRUE(loaded.absolutePath.string().find("HEAD:") != std::string::npos);
}

TEST(LoadUtf8TextFromBytes, InvalidUtf8IsUnsupported)
{
    const std::string bytes = std::string("x") + std::string(1, static_cast<char>(0xC3)) + "(";
    const auto loaded = LoadUtf8TextFromBytes(bytes, "HEAD:bin.dat");
    EXPECT_EQ(loaded.status, LoadStatus::NotUtf8);
    EXPECT_TRUE(IsUnsupportedText(loaded));
}

TEST(Utf8Validation, AcceptsAsciiAndMultibyte)
{
    EXPECT_TRUE(IsValidUtf8(""));
    EXPECT_TRUE(IsValidUtf8("hello"));

    // U+00A2 (¢): C2 A2
    EXPECT_TRUE(IsValidUtf8(std::string_view("\xC2\xA2", 2)));

    // U+20AC (€): E2 82 AC
    EXPECT_TRUE(IsValidUtf8(std::string_view("\xE2\x82\xAC", 3)));

    // U+1F600 (😀): F0 9F 98 80
    EXPECT_TRUE(IsValidUtf8(std::string_view("\xF0\x9F\x98\x80", 4)));
}

TEST(Utf8Validation, RejectsCommonInvalidSequences)
{
    // Lone continuation byte.
    EXPECT_FALSE(IsValidUtf8(std::string_view("\x80", 1)));

    // Truncated 2-byte sequence.
    EXPECT_FALSE(IsValidUtf8(std::string_view("\xC2", 1)));

    // Overlong encoding of '/': C0 AF (should be 2F).
    EXPECT_FALSE(IsValidUtf8(std::string_view("\xC0\xAF", 2)));

    // UTF-16 surrogate half U+D800 encoded in UTF-8 is invalid: ED A0 80.
    EXPECT_FALSE(IsValidUtf8(std::string_view("\xED\xA0\x80", 3)));

    // Codepoint > U+10FFFF: F4 90 80 80.
    EXPECT_FALSE(IsValidUtf8(std::string_view("\xF4\x90\x80\x80", 4)));
}

TEST(IsUnsupportedText, TrueOnlyForNotUtf8)
{
    LoadedTextFile f;
    f.status = LoadStatus::Ok;
    EXPECT_FALSE(IsUnsupportedText(f));
    f.status = LoadStatus::NotFound;
    EXPECT_FALSE(IsUnsupportedText(f));
    f.status = LoadStatus::Unreadable;
    EXPECT_FALSE(IsUnsupportedText(f));
    f.status = LoadStatus::NotUtf8;
    EXPECT_TRUE(IsUnsupportedText(f));
}

} // namespace bendiff::core
