#pragma once

#include <diff/diff.h>

#include <string>
#include <string_view>

namespace bendiff::core::diff {

// Produces a comparison key for a single line according to whitespace mode.
//
// Whitespace definition (v1): ASCII space ' ', tab '\t', and carriage return '\r'.
// (Lines are expected to be newline-split already, so '\n' should not be present.)
std::string MakeComparisonKey(std::string_view line, WhitespaceMode mode);

} // namespace bendiff::core::diff
