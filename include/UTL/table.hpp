// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::table
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_table.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTL_MODULE_TABLE)

#ifndef utl_table_headerguard
#define utl_table_headerguard

#define UTL_TABLE_VERSION_MAJOR 1
#define UTL_TABLE_VERSION_MINOR 0
#define UTL_TABLE_VERSION_PATCH 2

// _______________________ INCLUDES _______________________

#include <array>    // array<>, size_t
#include <cassert>  // assert()
#include <charconv> // to_chars()
#include <string>   // string
#include <vector>   // vector<>

// ____________________ DEVELOPER DOCS ____________________

// Supported table formats:
//    - ASCII
//    - Markdown
//    - LaTeX
//    - Mathematica
//    - CSV
//
// Those formats impose following assertions:
//    1. Table rows / columns should not have a fixed datatype
//    2. The notion of "header" is not present in a general case (it only matters for Markdown)
//    3. The notion of "hline" is not present in a general case (it only matters for LaTex & Mathematica)
//    4. Floating point number formatting depends on the table format
//
//    - Assertion  (1) means "variadic" designs with fixed-type columns are generally not suitable.
//    - Assertions (2) and (3) mean that it is difficult to generalize all formatting details.
//    - Assertion  (4) means that it is difficult to "delay" format specification, we'll either have
//    to re-parse number back from string or have some kind of dynamic typing that remembers numbers
//    (since assertion (1) prevents us form using a statically typed design).
//
// This motivates the end design where each format has it's own class, this means we can format things "eagerly"
// (thus avoiding issue (4)), different classes have independent (through similar) APIs and implementations
// (thus avoiding generalization problems (2) and (3)). There is certainly some code repetition, but it ends
// up being better than the alternative.
//
// Most of the common logic between formats is handled by the cell matrix class, which abstracts away all
// the indexation / appending / width computation and etc. behind a rather efficient API. Since eagerly
// constructing a table is effectively the same as building a matrix of strings, we avoid issue (1) without
// the need for dynamic typing.
//
// Note: We can reduce allocations even more by packing all strings into a single char vector, and
//       storing 'std::string_view's into it, cell matrix should be able to do so rather easily.
//       Whether such optimization is worth the trouble is debatable.

// ____________________ IMPLEMENTATION ____________________

namespace utl::table::impl {

// ======================
// --- SFINAE helpers ---
// ======================

template <bool Cond>
using require = std::enable_if_t<Cond, bool>; // makes SFINAE a bit less cumbersome

template <class T>
using require_int = require<std::is_integral_v<T> && !std::is_same_v<T, bool>>;

template <class T>
using require_bool = require<std::is_same_v<T, bool>>;

template <class T>
using require_float = require<std::is_floating_point_v<T>>;

template <class T>
using require_strconv = require<std::is_convertible_v<T, std::string>>;

// ====================
// --- String utils ---
// ====================

// Well-tested implementations taken from 'utl::stre'

[[nodiscard]] inline std::string trim_left(std::string_view str, char trimmed_char) {
    std::string res(str);
    res.erase(0, res.find_first_not_of(trimmed_char));
    return res;
}

template <class T>
[[nodiscard]] std::string replace_all_occurrences(T&& str, std::string_view from, std::string_view to) {
    std::string res = std::forward<T>(str);

    std::size_t i = 0;
    while ((i = res.find(from, i)) != std::string::npos) { // locate substring to replace
        res.replace(i, from.size(), to);                   // replace
        i += to.size();                                    // step over the replaced region
    }

    return res;
}

[[nodiscard]] inline std::string escape_control_chars(std::string_view str) {
    std::string res;
    res.reserve(str.size()); // not necessarily correct, but it's a good first guess

    for (const char c : str) {
        // Control characters with dedicated escape sequences get escaped with those sequences
        if (c == '\a') res += "\\a";
        else if (c == '\b') res += "\\b";
        else if (c == '\f') res += "\\f";
        else if (c == '\n') res += "\\n";
        else if (c == '\r') res += "\\r";
        else if (c == '\t') res += "\\t";
        else if (c == '\v') res += "\\v";
        // Other non-printable chars get replaced with their codes
        else if (!std::isprint(static_cast<unsigned char>(c))) {
            res += '\\';
            res += std::to_string(static_cast<int>(c));
        }
        // Printable chars are appended as is.
        else
            res += c;
    }
    // Note: This could be implemented faster using the 'utl::json' method of handling escapes with buffering and
    // a lookup table, however I don't see much practical reason to complicate this implementation like that.

    return res;
}

// ============================
// --- Number serialization ---
// ============================

// --- Typed wrapper ---
// ---------------------

// Thin wrapper around the floating point value used by tables to apply format-specific stringification
template <class T, require_float<T> = true>
struct Number {
    T                 value;
    std::chars_format format;
    int               precision;

    constexpr explicit Number(T value, std::chars_format format = std::chars_format::general,
                              int precision = 3) noexcept
        : value(value), format(format), precision(precision) {}
};

// --- Serialization ---
// ---------------------

// Note 1: 80-bit 'long double' fits in 29 chars, 64-bit 'double' fits in 24
// Note 2: 'std::uint64_t' fits in 21 chars, GCC '__uint128_t' fits in 40
// Note 3: 'to_chars()' can only fail due to a small buffer, no need to check errors release

template <class T, require_float<T> = true>
inline std::string to_chars_number(Number<T> number) {
    std::array<char, 30> buffer;

    const std::to_chars_result res =
        std::to_chars(buffer.data(), buffer.data() + buffer.size(), number.value, number.format, number.precision);
    assert(res.ec == std::errc{});

    return std::string(buffer.data(), res.ptr);
}

template <class T, require_float<T> = true>
inline std::string to_chars_float(T value) {
    std::array<char, 30> buffer;

    const std::to_chars_result res = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
    assert(res.ec == std::errc{});

    return std::string(buffer.data(), res.ptr);
}

template <class T, require_int<T> = true>
inline std::string to_chars_int(T value) {
    std::array<char, 40> buffer;

    const std::to_chars_result res = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
    assert(res.ec == std::errc{});

    return std::string(buffer.data(), res.ptr);
}

// --- LaTeX reformatting ---
// --------------------------

// LaTeX formatting wraps number into formula segment and rewrites scientific notation, below are a few examples:
//
//    - "1"      -> "$1$"
//    - "1e+2"   -> "$10^{2}$"
//    - "1.3e-3" -> "$1.3 \cdot 10^{-3}$"
//
// This should be done with string manipulation, since we cannot know whether the number serializes to regular or
// scientific form without intrusively integrating with the float formatting algorithm (which by itself is much more
// complex that this entire header). This is somewhat inefficient, but there is no practical reason to ever generate
// LaTeX tables large enough for it to become a bottleneck.

// Check that 'str' satisfies pattern '1[.][0...0]' (regex '1?.*0')
inline bool satisfies_pattern_of_one(std::string_view str) {
    std::size_t i = 0;

    while (i < 1 && str[i] == '1') ++i;
    while (i < 2 && str[i] == '.') ++i;
    while (i < str.size() && str[i] == '0') ++i;

    return i == str.size();
}

// Reformat numbers in scientific & hex notation
inline std::string latex_reformat(std::string_view str) {
    const std::size_t exp_idx       = str.find('e');
    const bool        is_scientific = (exp_idx != std::string::npos);
    const bool        is_hex        = (str.find('p') != std::string::npos);
    const bool        is_regular    = !is_scientific && !is_hex;

    if (is_regular) return std::string(str);               // regular form doesn't require reformatting
    if (is_hex) return "\\text{" + std::string(str) + "}"; // hex form has no better solution the to quote it "verbatim"

    const std::string_view mantissa = str.substr(0, exp_idx);
    const char             sign     = str.at(exp_idx + 1);
    const std::string_view exponent = str.substr(exp_idx + 2);

    std::string res;
    res.reserve(sizeof(" \\cdot 10^{}") + mantissa.size() + exponent.size());

    if (!satisfies_pattern_of_one(mantissa)) { // powers of 10 don't need the fractional part
        res += mantissa;
        res += " \\cdot ";
    }
    res += "10^{";
    if (sign == '-') res += sign;
    const std::string trimmed_exponent = trim_left(exponent, '0');
    res += trimmed_exponent.empty() ? "0" : trimmed_exponent; // prevents stuff like '10^{}'
    res += '}';

    return res;
}

// Wrap number in '$'
inline std::string latex_wrap(std::string str) { return '$' + std::move(str) + '$'; }

// --- Mathematica reformatting ---
// --------------------------------

// Mathematica uses "*^" instead of "e" to denote scientific notation

inline std::string mathematica_reformat(std::string str) { return replace_all_occurrences(std::move(str), "e", "*^"); }

// ====================
// --- Common utils ---
// ====================

// Avoids including '<algorithm>' for a single tiny function
[[nodiscard]] inline std::size_t max(std::size_t lhs, std::size_t rhs) noexcept { return lhs < rhs ? rhs : lhs; }

// Used in delimiter placement
[[nodiscard]] constexpr bool not_last(std::size_t i, std::size_t size) noexcept { return i < size - 1; }

// Used to align cell widths
inline void aligned_append(std::string& dst, const std::string& src, std::size_t width) {
    dst += src;
    dst.append(width - src.size(), ' ');
}

// Every format does its own thing, but they all need rows/cols/widths/etc.
// to do the alignment, makes sense to group all this stuff into a struct
struct Extents {
    std::size_t              rows;
    std::size_t              cols;
    std::vector<std::size_t> widths;        // useful for alignment
    std::size_t              total_width;   // useful for preallocation
    std::size_t              last_cell_row; // useful for delimiter placement
};

// ====================
// --- Table matrix ---
// ====================

// Any table can be represented as a matrix of strings,
// with some of the rows corresponding to hlines and not containing any content:
//
//    [false]  [ "text_00", "text_01", "text_02" ]
//    [ true]  [ ""       , ""       , ""        ]
//    [false]  [ "text_00", "text_01", "text_02" ]
//    [false]  [ "text_00", "text_01", "text_02" ]
//
// This is a pretty efficient way of packing the data (relative to the alternatives)
// that allows us to write table operations in a generic, yet concise manner.

class Matrix {
    std::size_t rows = 0;
    std::size_t cols = 0;

    std::vector<bool>        hlines; // [ rows ]        dense vector
    std::vector<std::string> cells;  // [ rows x cols ] dense matrix

public:
    // Matrix has a fixed number of cols and a growing number of rows,
    // constructed either using a number of cols or the first row data
    explicit Matrix(std::size_t cols) noexcept : cols(cols) { assert(this->cols > 0); }
    explicit Matrix(std::vector<std::string> title) : rows(1), cols(title.size()) {
        assert(this->cols > 0);

        this->hlines.push_back(false);
        this->cells = std::move(title);
    }

    // Creating regular rows (happens on cell-by-cell basis)
    void add_cell(std::string cell) {
        this->cells.push_back(std::move(cell));

        // New row creation
        if (this->cells.size() > this->rows * this->cols) {
            ++this->rows;
            this->hlines.push_back(false);
        }
    }

    // Creating hlines
    void add_hline() {
        this->normalize();
        // fills current row with empty cells if it's not finished, note that this approach
        // of hlines filling their matrix row with empty cells is what allows other functions to
        // have generic logic without constantly checking for hlines

        ++this->rows;
        this->hlines.push_back(true);
        this->cells.resize(this->cells.size() + this->cols);
    }

    // Normalizing matrix to a rectangular form
    // (fills possibly "unfinished" data with empty cells to make the matrix rectangular)
    // (returns self-reference to allow operation chaining)
    Matrix& normalize() {
        this->cells.resize(this->rows * this->cols);
        return *this;
    }

    // Element access
    [[nodiscard]] bool get_hline(std::size_t i) const { return this->hlines.at(i); }

    [[nodiscard]] const std::string& get_cell(std::size_t i, std::size_t j) const {
        assert(this->cells.size() == this->rows * this->cols); // matrix-like access assumes rectangular form

        return this->cells.at(i * this->cols + j);
    }

    // Extent counting
    [[nodiscard]] Extents get_extents() const {
        // Individual column widths
        std::vector<std::size_t> widths(this->cols, 0);

        for (std::size_t i = 0; i < this->rows; ++i)
            for (std::size_t j = 0; j < this->cols; ++j) widths[j] = max(widths[j], this->get_cell(i, j).size());

        // Total row width
        std::size_t total_width = 0;
        for (const auto& width : widths) total_width += width;

        // Last cell row
        std::size_t last_cell_row = this->rows - 1;
        while (last_cell_row > 0 && this->get_hline(last_cell_row)) --last_cell_row;

        return {this->rows, this->cols, std::move(widths), total_width, last_cell_row};
    }

    [[nodiscard]] bool ended_on_hline() const { return !this->hlines.empty() && this->hlines.back(); }
    // some formats need to check for consequent hlines
};

// =====================
// --- Format: ASCII ---
// =====================

// Title row -> NO
// Hlines    -> YES
// Numbers   -> REGULAR

// ASCII tables are usually printed to terminal or a file,

class ASCII {
    Matrix matrix;

public:
    explicit ASCII(std::size_t cols) : matrix(cols) {}

    template <class T, require_float<T> = true>
    void cell(Number<T> value) {
        this->matrix.add_cell(to_chars_number(value));
    }

    template <class T, require_float<T> = true>
    void cell(T value) {
        this->matrix.add_cell(to_chars_float(value));
    }

    template <class T, require_int<T> = true>
    void cell(T value) {
        this->matrix.add_cell(to_chars_int(value));
    }

    template <class T, require_bool<T> = true>
    void cell(T value) {
        this->matrix.add_cell(value ? "true" : "false");
    }

    template <class T, require_strconv<T> = true>
    void cell(T value) {
        this->matrix.add_cell(escape_control_chars(value));
    }

    template <class... T, require<sizeof...(T) != 1> = true>
    void cell(T&&... args) {
        (this->cell(std::forward<T>(args)), ...);
    }

    void hline() { this->matrix.add_hline(); }

    std::string format() {
        const auto extents = this->matrix.normalize().get_extents();

        // Preallocate string considering ASCII row format: "| " ... " | " ... " |\n"
        std::string res;
        res.reserve(extents.rows * (extents.total_width + 3 * extents.cols));

        // Formatting functions
        const auto format_row = [&](std::size_t i) {
            res += "| ";
            for (std::size_t j = 0; j < extents.cols; ++j) {
                aligned_append(res, this->matrix.get_cell(i, j), extents.widths[j]);
                if (not_last(j, extents.cols)) res += " | ";
            }
            res += " |\n";
        };

        const auto format_hline = [&] {
            res += '|';
            for (std::size_t j = 0; j < extents.cols; ++j) {
                res.append(extents.widths[j] + 2, '-');
                if (not_last(j, extents.cols)) res += '|';
            }
            res += "|\n";
        };

        // Format table with hlines
        for (std::size_t i = 0; i < extents.rows; ++i)
            if (this->matrix.get_hline(i)) format_hline();
            else format_row(i);

        return res;
    }
};

// ========================
// --- Format: Markdown ---
// ========================

// Title row -> YES
// Hlines    -> NO
// Numbers   -> REGULAR

// Markdown is implementation-defined so we format tables in a commonly supported way and don't impose
// any specific restrictions on strings allowed in a cell (for example, some markdown flavors might
// want to export HTML cells, while other would consider such syntax to be invalid).

class Markdown {
    Matrix matrix;

public:
    // Every markdown table has precisely one title row, making it a constructor argument (rather than a
    // '.title()' method) ensures this fact at the API level and saves us from a bunch of pointless checks
    explicit Markdown(std::vector<std::string> title) : matrix(std::move(title)) {}

    template <class T, require_float<T> = true>
    void cell(Number<T> value) {
        this->matrix.add_cell(to_chars_number(value));
    }

    template <class T, require_float<T> = true>
    void cell(T value) {
        this->matrix.add_cell(to_chars_float(value));
    }

    template <class T, require_int<T> = true>
    void cell(T value) {
        this->matrix.add_cell(to_chars_int(value));
    }

    template <class T, require_bool<T> = true>
    void cell(T value) {
        this->matrix.add_cell(value ? "`true`" : "`false`");
    }

    template <class T, require_strconv<T> = true>
    void cell(T value) {
        this->matrix.add_cell(value);
    }

    template <class... T, require<sizeof...(T) != 1> = true>
    void cell(T&&... args) {
        (this->cell(std::forward<T>(args)), ...);
    }

    std::string format() {
        const auto extents = this->matrix.normalize().get_extents();

        // Preallocate string considering markdown row format: "| " ... " | " ... " |\n"
        std::string res;
        res.reserve(extents.rows * (extents.total_width + 3 * extents.cols));

        // Formatting functions
        const auto format_row = [&](std::size_t i) {
            res += "| ";
            for (std::size_t j = 0; j < extents.cols; ++j) {
                aligned_append(res, this->matrix.get_cell(i, j), extents.widths[j]);
                if (not_last(j, extents.cols)) res += " | ";
            }
            res += " |\n";
        };

        const auto format_hline = [&] {
            res += "| ";
            for (std::size_t j = 0; j < extents.cols; ++j) {
                res.append(extents.widths[j], '-');
                if (not_last(j, extents.cols)) res += " | ";
            }
            res += " |\n";
        };

        // Format table with title & body
        format_row(0);
        format_hline();
        for (std::size_t i = 1; i < extents.rows; ++i) format_row(i);

        return res;
    }
};

// =====================
// --- Format: LaTeX ---
// =====================

// Title row -> NO
// Hlines    -> YES
// Numbers   -> LaTeX-specific

// LaTeX is a little cumbersome to generate since we need to rewrite numbers in scientific form as formulas.
// Strings intentionally don't escape any special chars to allow users to write LaTeX expressions in string cells.

class LaTeX {
    Matrix matrix;

public:
    explicit LaTeX(std::size_t cols) : matrix(cols) {}

    template <class T, require_float<T> = true>
    void cell(Number<T> value) {
        this->matrix.add_cell(latex_wrap(latex_reformat(to_chars_number(value))));
    }

    template <class T, require_float<T> = true>
    void cell(T value) {
        this->matrix.add_cell(latex_wrap(latex_reformat(to_chars_float(value))));
    }

    template <class T, require_int<T> = true>
    void cell(T value) {
        this->matrix.add_cell(latex_wrap(to_chars_int(value)));
    }

    template <class T, require_bool<T> = true>
    void cell(T value) {
        this->matrix.add_cell(value ? "true" : "false");
    }

    template <class T, require_strconv<T> = true>
    void cell(T value) {
        this->matrix.add_cell(value);
    }

    template <class... T, require<sizeof...(T) != 1> = true>
    void cell(T&&... args) {
        (this->cell(std::forward<T>(args)), ...);
    }

    void hline() { this->matrix.add_hline(); }

    std::string format() {
        const auto extents = this->matrix.normalize().get_extents();

        // Preallocate string considering LaTeX environment and row format
        std::string res;
        res.reserve(17 + extents.cols * 2 + 2 + extents.rows * (extents.total_width + 3 * extents.cols) + 14);

        // Formatting functions
        const auto format_row = [&](std::size_t i) {
            res += "    ";
            for (std::size_t j = 0; j < extents.cols; ++j) {
                aligned_append(res, this->matrix.get_cell(i, j), extents.widths[j]);
                if (not_last(j, extents.cols)) res += " & ";
            }
            if (not_last(i, extents.rows)) res += " \\\\";
            res += '\n';
        };

        const auto format_hline = [&] { res += "\\hline\n"; };

        // Format table with hlines as LaTeX environment '\begin{tabular}{...} ... \end{tabular}'
        res += "\\begin{tabular}{|";
        for (std::size_t j = 0; j < extents.cols; ++j) res += "c|";
        res += "}\n";

        for (std::size_t i = 0; i < extents.rows; ++i)
            if (this->matrix.get_hline(i)) format_hline();
            else format_row(i);

        res += "\\end{tabular}\n";

        return res;
    }
};

// ===========================
// --- Format: Mathematica ---
// ===========================

// Title row -> NO
// Hlines    -> YES
// Numbers   -> Mathematica-specific

// Mathematica 'Grid[]' is not a format with a standardized specification, nonetheless it is often quite useful
// to print numerical params when visualizing numeric results. Wolfram strings seem to support newlines and most
// control characters out of the box, quotes can be escaped with '\"'.

class Mathematica {
    Matrix matrix;

public:
    explicit Mathematica(std::size_t cols) : matrix(cols) {}

    template <class T, require_float<T> = true>
    void cell(Number<T> value) {
        this->matrix.add_cell(mathematica_reformat(to_chars_number(value)));
    }

    template <class T, require_float<T> = true>
    void cell(T value) {
        this->matrix.add_cell(mathematica_reformat(to_chars_float(value)));
    }

    template <class T, require_int<T> = true>
    void cell(T value) {
        this->matrix.add_cell(to_chars_int(value));
    }

    template <class T, require_bool<T> = true>
    void cell(T value) {
        this->matrix.add_cell(value ? "True" : "False");
        // Mathematica capitalizes booleans
    }

    template <class T, require_strconv<T> = true>
    void cell(T value) {
        this->matrix.add_cell('"' + replace_all_occurrences(std::string(value), "\"", "\\\"") + '"');
        // without quotes Mathematica would interpret strings as identifiers
    }

    template <class... T, require<sizeof...(T) != 1> = true>
    void cell(T&&... args) {
        (this->cell(std::forward<T>(args)), ...);
    }

    void hline() {
        if (this->matrix.ended_on_hline()) return;
        // Mathematica cannot have multiple hlines in a row, we can ignore multiple consequent cases
        this->matrix.add_hline();
    }

    std::string format() {
        const auto extents = this->matrix.normalize().get_extents();

        // Preallocate string considering Mathematica syntax and row format
        std::string res;
        res.reserve(7 + extents.rows * (6 + extents.total_width + 2 * extents.cols + 4) + 22 + extents.cols * 8 + 4);

        // Formatting functions
        const auto format_row = [&](std::size_t i) {
            res += "    { ";
            for (std::size_t j = 0; j < extents.cols; ++j) {
                aligned_append(res, this->matrix.get_cell(i, j), extents.widths[j]);
                if (not_last(j, extents.cols)) res += ", ";
            }
            res += " }";
            if (i != extents.last_cell_row) res += ',';
            res += '\n';
        };

        const auto format_hline = [&](std::size_t i) {
            res += this->matrix.get_hline(i) ? "True" : "False";
            if (not_last(i, extents.rows)) res += ", ";
        };

        // Format table as Mathematica 'Grid[]' with hlines as 'Dividers ->' option,
        // for example: 'Grid[{...}, Dividers -> {All, {True, True, False, False, True}}]'
        //
        // Note that hline placement is somewhat non-trivial, 'Dividers' corresponds to exactly 'cell_rows + 1'
        // upper/lower edges and lines between rows, but the resulting algorithm ends up quite simple
        res += "Grid[{\n";

        for (std::size_t i = 0; i < extents.rows; ++i)
            if (!this->matrix.get_hline(i)) format_row(i);

        res += "}, Dividers -> {All, {";
        for (std::size_t i = 0; i < extents.rows; ++i) {
            format_hline(i);
            if (this->matrix.get_hline(i)) ++i;
        }
        res += "}}]\n";

        return res;
    }
};

// ===================
// --- Format: CSV ---
// ===================

// Title row -> NO
// Hlines    -> NO
// Numbers   -> REGULAR

// CSV is a format with no standard specification. As per RFC-4180 (see https://www.rfc-editor.org/info/rfc4180):
//    "While there are various specifications and implementations for the CSV format (for ex. ...), there is
//    no formal specification in existence, which allows for a wide variety of interpretations of CSV files.
//    This section documents the format that seems to be followed by most implementations."
// This implementation complies with requirements posed by RFC.

class CSV {
    Matrix matrix;

public:
    explicit CSV(std::size_t cols) : matrix(cols) {}

    template <class T, require_float<T> = true>
    void cell(Number<T> value) {
        this->matrix.add_cell(to_chars_number(value));
    }

    template <class T, require_float<T> = true>
    void cell(T value) {
        this->matrix.add_cell(to_chars_float(value));
    }

    template <class T, require_int<T> = true>
    void cell(T value) {
        this->matrix.add_cell(to_chars_int(value));
    }

    template <class T, require_bool<T> = true>
    void cell(T value) {
        this->matrix.add_cell(value ? "true" : "false");
    }

    template <class T, require_strconv<T> = true>
    void cell(T value) {
        this->matrix.add_cell('"' + std::string(value) + '"');
        // RFC-4180 [2.5] CSV should wrap strings that potentially contain commas, newline and quotes in '"'
    }

    template <class... T, require<sizeof...(T) != 1> = true>
    void cell(T&&... args) {
        (this->cell(std::forward<T>(args)), ...);
    }

    std::string format() {
        const auto extents = this->matrix.normalize().get_extents();

        // Preallocate string considering CSV row format: ... ", " ... ",\n"
        std::string res;
        res.reserve(extents.rows * (extents.total_width + 2 * extents.cols));

        // Formatting functions
        const auto format_row = [&](std::size_t i) {
            for (std::size_t j = 0; j < extents.cols; ++j) {
                res += this->matrix.get_cell(i, j);
                if (not_last(j, extents.cols)) res += ",";
                // RFC-4180 [2.4] No alignment since CSV doesn't ignore spaces and considers them a part of the field
            }
            res += '\n';
        };

        // Format table as a comma-separated list
        for (std::size_t i = 0; i < extents.rows; ++i) format_row(i);

        return res;
    }
};

} // namespace utl::table::impl

// ______________________ PUBLIC API ______________________

namespace utl::table {

using impl::Number;

using impl::ASCII;
using impl::Markdown;
using impl::LaTeX;
using impl::Mathematica;
using impl::CSV;

} // namespace utl::table

#endif
#endif // module utl::table
