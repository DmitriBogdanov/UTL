// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::stre
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_stre.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTL_MODULE_STRE)

#ifndef utl_stre_headerguard
#define utl_stre_headerguard

#define UTL_STRE_VERSION_MAJOR 2
#define UTL_STRE_VERSION_MINOR 0
#define UTL_STRE_VERSION_PATCH 0

// _______________________ INCLUDES _______________________

#include <array>       // array<>, size_t
#include <climits>     // CHAR_BIT
#include <cstdint>     // uint8_t
#include <string>      // string
#include <string_view> // string_view
#include <vector>      // vector<>

// ____________________ DEVELOPER DOCS ____________________

// String utils. Nothing fancy, basic stuff, however there is a lot of really bad implementations
// posted online, which is why I'd rather put an effort to get them right once and be done with it.
//
// Functions that can reuse the storage of an 'r-value' argument take 'std::string' by value,
// otherwise we taking 'std::string_view' is the most sensible way.

// ____________________ IMPLEMENTATION ____________________

namespace utl::stre::impl {

// ================================
// --- Character classification ---
// ================================

// This is effectively a sane reimplementation of character classification from <cctype>. Standard <cctype> functions
// don't operate on 'char' parameters as would be intuitive, instead they accept 'int' parameters which should be
// representable as 'unsigned char' (invoking UB otherwise), this makes them extremely error-prone in common use cases.
//
// Here we reimplement the exact same classification using a few lookup tables, performance shouldn't
// be meaningfully different and we get additional 'constexpr' and 'noexcept' guarantees.
//
// See https://en.cppreference.com/w/cpp/header/cctype.html

// --- Lookup tables ---
// ---------------------

static_assert(CHAR_BIT == 8); // weird platforms might need a different lookup table size

[[nodiscard]] constexpr std::uint8_t u8(char value) noexcept { return static_cast<std::uint8_t>(value); }

template <class Predicate>
[[nodiscard]] constexpr std::array<bool, 256> predicate_lookup_table(Predicate&& predicate) noexcept {
    std::array<bool, 256> res{};
    for (std::size_t i = 0; i < res.size(); ++i) res[i] = predicate(static_cast<char>(i));
    return res;
}

// clang-format off
constexpr auto lookup_digit            = predicate_lookup_table([](char ch) { return '0' <= ch && ch <= '9'; });
constexpr auto lookup_lowercase_letter = predicate_lookup_table([](char ch) { return 'a' <= ch && ch <= 'z'; });
constexpr auto lookup_uppercase_letter = predicate_lookup_table([](char ch) { return 'A' <= ch && ch <= 'Z'; });
constexpr auto lookup_punctuation      = predicate_lookup_table([](char ch) {
    return std::string_view{R"(!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~)"}.find_first_of(ch) != std::string_view::npos;
});
constexpr auto lookup_hexadecimal      = predicate_lookup_table([](char ch) {
    return std::string_view{R"(0123456789ABCDEFabcdef)"}.find_first_of(ch) != std::string_view::npos;
});
constexpr auto lookup_space            = predicate_lookup_table([](char ch) {
    return std::string_view{" \f\n\r\t\v"}.find_first_of(ch) != std::string_view::npos;
});
constexpr auto lookup_control          = predicate_lookup_table([](char ch) {
    return (0x00 <= static_cast<int>(ch) && static_cast<int>(ch) <= 0x1F) || static_cast<int>(ch) == 0x7F;
});
// clang-format on

// --- Public API ---
// ------------------

// clang-format off
[[nodiscard]] constexpr bool is_digit       (char ch) noexcept { return lookup_digit[u8(ch)];                      }
[[nodiscard]] constexpr bool is_lowercase   (char ch) noexcept { return lookup_lowercase_letter[u8(ch)];           }
[[nodiscard]] constexpr bool is_uppercase   (char ch) noexcept { return lookup_uppercase_letter[u8(ch)];           }
[[nodiscard]] constexpr bool is_punctuation (char ch) noexcept { return lookup_punctuation[u8(ch)];                }
[[nodiscard]] constexpr bool is_hexadecimal (char ch) noexcept { return lookup_hexadecimal[u8(ch)];                }
[[nodiscard]] constexpr bool is_control     (char ch) noexcept { return lookup_control[u8(ch)];                    }
[[nodiscard]] constexpr bool is_alphabetic  (char ch) noexcept { return is_lowercase(ch) || is_uppercase(ch);      }
[[nodiscard]] constexpr bool is_alphanumeric(char ch) noexcept { return is_digit(ch) || is_alphabetic(ch);         }
[[nodiscard]] constexpr bool is_graphical   (char ch) noexcept { return is_alphanumeric(ch) || is_punctuation(ch); }
[[nodiscard]] constexpr bool is_printable   (char ch) noexcept { return is_graphical(ch) || (ch == ' ');           }
[[nodiscard]] constexpr bool is_space       (char ch) noexcept { return lookup_space[u8(ch)];                      }
[[nodiscard]] constexpr bool is_blank       (char ch) noexcept { return ch == ' ' || ch == '\t';                   }
// clang-format on

// ========================
// --- Case conversions ---
// ========================

// Note: This is a locale-independent ASCII-only conversion, Unicode conversion would
//       require a full-fledged grapheme parsing with language-specific conversion schemas.

[[nodiscard]] constexpr char to_lower(char ch) noexcept {
    constexpr char offset = 'z' - 'Z';
    return ('A' <= ch && ch <= 'Z') ? ch + offset : ch;
}

[[nodiscard]] constexpr char to_upper(char ch) noexcept {
    constexpr char offset = 'Z' - 'z';
    return ('a' <= ch && ch <= 'z') ? ch + offset : ch;
}

[[nodiscard]] inline std::string to_lower(std::string str) {
    for (auto& ch : str) ch = to_lower(ch);
    return str;
}

[[nodiscard]] inline std::string to_upper(std::string str) {
    for (auto& ch : str) ch = to_upper(ch);
    return str;
}

// ================
// --- Trimming ---
// ================

// Note: Unlike most mutating operations which require an allocated
//       result, trimming can be done on views and at compile-time.

// --- std::string_view ---
// ------------------------

[[nodiscard]] constexpr std::string_view trim_left(std::string_view str, char trimmed_char = ' ') noexcept {
    if (const std::size_t i = str.find_first_not_of(trimmed_char); i != std::string_view::npos) {
        str.remove_prefix(i);
        return str;
    } else {
        return std::string_view{}; // 'str' consists entirely of 'trimmed_char'
    }
}

[[nodiscard]] constexpr std::string_view trim_right(std::string_view str, char trimmed_char = ' ') noexcept {
    if (const std::size_t i = str.find_last_not_of(trimmed_char); i != std::string_view::npos) {
        str.remove_suffix(str.size() - i - 1);
        return str;
    } else {
        return std::string_view{}; // 'str' consists entirely of 'trimmed_char'
    }
}

[[nodiscard]] constexpr std::string_view trim(std::string_view str, char trimmed_char = ' ') noexcept {
    return trim_right(trim_left(str, trimmed_char), trimmed_char);
}

// --- C-string ---
// ----------------

[[nodiscard]] constexpr std::string_view trim_left(const char* str, char trimmed_char = ' ') noexcept {
    return trim_left(std::string_view{str}, trimmed_char);
}

[[nodiscard]] constexpr std::string_view trim_right(const char* str, char trimmed_char = ' ') noexcept {
    return trim_right(std::string_view{str}, trimmed_char);
}

[[nodiscard]] constexpr std::string_view trim(const char* str, char trimmed_char = ' ') noexcept {
    return trim(std::string_view{str}, trimmed_char);
}

// --- std::string ---
// -------------------

[[nodiscard]] inline std::string trim_left(std::string str, char trimmed_char = ' ') {
    str.erase(0, str.find_first_not_of(trimmed_char));
    return str;
}

[[nodiscard]] inline std::string trim_right(std::string str, char trimmed_char = ' ') {
    str.erase(str.find_last_not_of(trimmed_char) + 1);
    return str;
}

[[nodiscard]] inline std::string trim(std::string str, char trimmed_char = ' ') {
    return trim_right(trim_left(std::move(str), trimmed_char), trimmed_char);
}

// ===============
// --- Padding ---
// ===============

[[nodiscard]] inline std::string pad_left(std::string_view str, std::size_t length, char padding_char = ' ') {
    if (length > str.size()) {
        std::string res;
        res.reserve(length);
        res.append(length - str.size(), padding_char);
        res += str;
        return res;
    } else return std::string(str);
}

[[nodiscard]] inline std::string pad_right(std::string_view str, std::size_t length, char padding_char = ' ') {
    if (length > str.size()) {
        std::string res;
        res.reserve(length);
        res += str;
        res.append(length - str.size(), padding_char);
        return res;
    } else return std::string(str);
}

[[nodiscard]] inline std::string pad(std::string_view str, std::size_t length, char padding_char = ' ') {
    if (length > str.size()) {
        std::string res;
        res.reserve(length);
        const std::size_t left_pad_size = (length - str.size()) / 2;
        res.append(left_pad_size, padding_char);
        res += str;
        const std::size_t right_pad_size = length - str.size() - left_pad_size;
        res.append(right_pad_size, padding_char);
        return res;
        // we try to pad evenly on both sides, but one of the pads (the right one to be exact)
        // may be a character longer than the other if the length difference is odd
    } else return std::string(str);
}

// ========================
// --- Substring checks ---
// ========================

// Note: C++20 standardizes the same functionality through 'std::basic_string<T>::starts_with()',
//       'std::basic_string<T>::ends_with()' and 'std::basic_string<T>::contains()'.

[[nodiscard]] constexpr bool starts_with(std::string_view str, std::string_view substr) noexcept {
    return str.size() >= substr.size() && str.compare(0, substr.size(), substr) == 0;
}

[[nodiscard]] constexpr bool ends_with(std::string_view str, std::string_view substr) noexcept {
    return str.size() >= substr.size() && str.compare(str.size() - substr.size(), substr.size(), substr) == 0;
}

[[nodiscard]] constexpr bool contains(std::string_view str, std::string_view substr) noexcept {
    return str.find(substr) != std::string_view::npos;
}

// =============================
// --- Substring replacement ---
// =============================

[[nodiscard]] inline std::string replace_all(std::string str, std::string_view from, std::string_view to) {
    std::size_t i = 0;
    while ((i = str.find(from, i)) != std::string::npos) { // locate substring to replace
        str.replace(i, from.size(), to);                   // replace
        i += to.size();                                    // step over the replaced region
    }
    // Note: Not stepping over the replaced regions causes self-similar replacements
    // like "123" -> "123123" to fall into an infinite loop, we don't want that.

    return str;
}

[[nodiscard]] inline std::string replace_first(std::string str, std::string_view from, std::string_view to) {
    if (const std::size_t i = str.find(from); i != std::string::npos) str.replace(i, from.size(), to);
    return str;
}

[[nodiscard]] inline std::string replace_last(std::string str, std::string_view from, std::string_view to) {
    if (const std::size_t i = str.rfind(from); i != std::string::npos) str.replace(i, from.size(), to);
    return str;
}

[[nodiscard]] inline std::string replace_prefix(std::string str, std::string_view from, std::string_view to) {
    if (starts_with(str, from)) str.replace(0, from.size(), to);
    return str;
}

[[nodiscard]] inline std::string replace_suffix(std::string str, std::string_view from, std::string_view to) {
    if (ends_with(str, from)) str.replace(str.size() - from.size(), from.size(), to);
    return str;
}

// =================
// --- Repeating ---
// =================

[[nodiscard]] inline std::string repeat(char ch, std::size_t repeats) { return std::string(repeats, ch); }

[[nodiscard]] inline std::string repeat(std::string_view str, std::size_t repeats) {
    std::string res;
    res.reserve(str.size() * repeats);
    for (std::size_t i = 0; i < repeats; ++i) res += str;
    return res;
}

// ================
// --- Escaping ---
// ================

// Escaping is quite useful when we need to print string to the terminal or
// serialize it to a file without special characters getting in the way.

[[nodiscard]] inline std::string escape(char ch) {
    if (!is_printable(ch)) {
        // Control characters with dedicated escape sequences get escaped with those sequences
        if (ch == '\n') return "\\n";
        else if (ch == '\t') return "\\t";
        else if (ch == '\r') return "\\r";
        else if (ch == '\f') return "\\f";
        else if (ch == '\a') return "\\a";
        else if (ch == '\b') return "\\b";
        else if (ch == '\v') return "\\v";
        // Other non-printable chars get replaced with their codes
        else return "\\" + std::to_string(static_cast<int>(ch));
    }

    return std::string(1, ch);
}

[[nodiscard]] inline std::string escape(std::string_view str) {
    const std::size_t result_size_heuristic = static_cast<std::size_t>(str.size() * 1.15);
    // we don't know how much to reserve exactly, but we can make a decent first guess

    std::string res;
    res.reserve(result_size_heuristic);

    // Since appending individual characters is ~twice as slow as appending the whole string, we
    // use a "buffered" way of appending, appending whole segments up to the currently escaped char.
    // This technique is typical for parsing/serialization libs and was benchmarked in 'utl::json'.
    std::size_t segment_start = 0;
    for (std::size_t i = 0; i < str.size(); ++i) {
        const char ch = str[i];

        if (!is_printable(ch)) {
            res.append(str.data() + segment_start, i - segment_start);
            res += '\\';
            
            segment_start = i + 1; // '+1' skips over the escaped character

            // Control characters with dedicated escape sequences get escaped with those sequences
            if (ch == '\n') res += 'n';
            else if (ch == '\t') res += 't';
            else if (ch == '\r') res += 'r';
            else if (ch == '\f') res += 'f';
            else if (ch == '\a') res += 'a';
            else if (ch == '\b') res += 'b';
            else if (ch == '\v') res += 'v';
            // Other non-printable chars get replaced with their codes
            else res += std::to_string(static_cast<int>(ch));

            // Note 1: Most common cases (newlines and tabs) are checked first
            // Note 2: 'std::to_string()' always fits into SSO for our range, no allocation is happening
        }
    }
    res.append(str.data() + segment_start, str.size() - segment_start);

    return res;
}

// ====================
// --- Tokenization ---
// ====================

// Most "split by delimiter" implementations found online seem to be quite inefficient
// with unnecessary copying/erasure/intermediate tokens, stringstreams and etc.
//
// We can just scan through the string view once, while keeping track of the last segment between
// two delimiters, no unnecessary work is performed, the only place where we do a copy is during
// emplacement into the result vector where it is unavoidable.
//
// 'tokenize()' => ignores   empty tokens
// 'split()'    => preserves empty tokens

[[nodiscard]] inline std::vector<std::string> tokenize(std::string_view str, std::string_view delimiter) {
    if (delimiter.empty()) return {std::string(str)};
    // handle empty delimiter explicitly so we can't fall into an infinite loop

    std::vector<std::string> tokens;
    std::size_t              cursor        = 0;
    std::size_t              segment_start = cursor;

    while ((cursor = str.find(delimiter, cursor)) != std::string_view::npos) {
        if (segment_start != cursor) tokens.emplace_back(str.substr(segment_start, cursor - segment_start));
        // don't emplace empty tokens in case of leading/trailing/repeated delimiter

        cursor += delimiter.size();
        segment_start = cursor;
    }

    if (segment_start != str.size()) tokens.emplace_back(str.substr(segment_start));
    // 'cursor' is now at 'npos', so we compare to the size instead

    return tokens;
}

[[nodiscard]] inline std::vector<std::string> split(std::string_view str, std::string_view delimiter) {
    if (delimiter.empty()) return {std::string(str)};
    // handle empty delimiter explicitly so we can't fall into an infinite loop

    std::vector<std::string> tokens;
    std::size_t              cursor        = 0;
    std::size_t              segment_start = cursor;

    while ((cursor = str.find(delimiter, cursor)) != std::string_view::npos) {
        tokens.emplace_back(str.substr(segment_start, cursor - segment_start));
        // possibly an empty token in the case of repeated/trailing delimiters

        cursor += delimiter.size();
        segment_start = cursor;
    }

    tokens.emplace_back(str.substr(segment_start)); // possibly an empty token

    return tokens;
}

// ==============================
// --- Difference measurement ---
// ==============================

[[nodiscard]] constexpr std::size_t first_difference(std::string_view lhs, std::string_view rhs) noexcept {
    // clang-format off
    if (lhs.size() < rhs.size()) {
        for (std::size_t i = 0; i < lhs.size(); ++i) if (lhs[i] != rhs[i]) return i;
        return lhs.size();
    }
    if (lhs.size() > rhs.size()) {
        for (std::size_t i = 0; i < rhs.size(); ++i) if (lhs[i] != rhs[i]) return i;
        return rhs.size();
    }
    if (lhs.size() == rhs.size()) {
        for (std::size_t i = 0; i < lhs.size(); ++i) if (lhs[i] != rhs[i]) return i;
        return std::string_view::npos;
    }
    // clang-format on

    return std::string_view::npos; // unreachable, necessary to avoid warnings
}

constexpr std::size_t count_difference(std::string_view lhs, std::string_view rhs) noexcept {
    std::size_t difference = 0;

    // clang-format off
    if (lhs.size() < rhs.size()) {
        for (std::size_t i = 0; i < lhs.size(); ++i) if (lhs[i] != rhs[i]) ++difference;
        return difference + rhs.size() - lhs.size();
    }
    if (lhs.size() > rhs.size()) {
        for (std::size_t i = 0; i < rhs.size(); ++i) if (lhs[i] != rhs[i]) ++difference;
        return difference + lhs.size() - rhs.size();
    }
    if (lhs.size() == rhs.size()) {
        for (std::size_t i = 0; i < lhs.size(); ++i) if (lhs[i] != rhs[i]) ++difference;
        return difference;
    }
    // clang-format on

    return difference; // unreachable, necessary to avoid warnings
}

} // namespace utl::stre::impl

// ______________________ PUBLIC API ______________________

namespace utl::stre {

using impl::is_digit;
using impl::is_lowercase;
using impl::is_uppercase;
using impl::is_punctuation;
using impl::is_hexadecimal;
using impl::is_control;
using impl::is_alphabetic;
using impl::is_alphanumeric;
using impl::is_graphical;
using impl::is_printable;
using impl::is_space;
using impl::is_blank;

using impl::to_lower;
using impl::to_upper;

using impl::trim_left;
using impl::trim_right;
using impl::trim;

using impl::pad_left;
using impl::pad_right;
using impl::pad;

using impl::starts_with;
using impl::ends_with;
using impl::contains;

using impl::replace_all;
using impl::replace_first;
using impl::replace_last;
using impl::replace_prefix;
using impl::replace_suffix;

using impl::repeat;

using impl::escape;

using impl::tokenize;
using impl::split;

using impl::first_difference;
using impl::count_difference;

} // namespace utl::stre

#endif
#endif // module utl::stre
