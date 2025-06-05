// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::table
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_table.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_TABLE)
#ifndef UTLHEADERGUARD_TABLE
#define UTLHEADERGUARD_TABLE

#define UTL_TABLE_VERSION_MAJOR 0 // [!] module awaiting a rewrite
#define UTL_TABLE_VERSION_MINOR 1
#define UTL_TABLE_VERSION_PATCH 0

// _______________________ INCLUDES _______________________

#include <cstddef>          // size_t
#include <initializer_list> // initializer_list<>
#include <iomanip>          // resetiosflags(), setw()
#include <ios>              // streamsize, ios_base::fmtflags, ios
#include <iostream>         // cout
#include <ostream>          // ostream
#include <sstream>          // ostringstream
#include <string>           // string
#include <type_traits>      // is_arithmetic_v<>, is_same_v<>
#include <vector>           // vector<>

// ____________________ DEVELOPER DOCS ____________________

// Functions used to build and render simple ASCII table in console.
//
// Tries to be simple and minimize boilerplate, exposes a LaTeX-like API.
// In fact this is for a reason - these tables can be formatted for a quick LaTeX export
// by enabling a 'set_latex_mode(true)'.
//
// As of now the implementation is short but quite frankly ugly, it feels like with some thought
// it could be generalized much better to support multiple styles and perform faster, however there
// is ~0 need for this to be fast since it's mean for human-readable tables and not massive data export.
// Adding more styles while nice also doesn't seem like an important thing as of now so the old implementation
// is left to be as is.

// ____________________ IMPLEMENTATION ____________________

namespace utl::table::impl {

// =====================
// --- Column Format ---
// =====================

using uint      = std::streamsize;
using ios_flags = std::ios_base::fmtflags;

struct ColumnFormat {
    ios_flags flags;
    uint      precision;
};

struct Column {
    uint         width;
    ColumnFormat col_format;
};

// --- Predefined Formats ---
// --------------------------

constexpr ColumnFormat NONE = {std::ios::showpoint, 6};

constexpr ColumnFormat FIXED(uint decimals = 3) { return {std::ios::fixed, decimals}; }
constexpr ColumnFormat DEFAULT(uint decimals = 6) { return {std::ios::showpoint, decimals}; }
constexpr ColumnFormat SCIENTIFIC(uint decimals = 3) { return {std::ios::scientific, decimals}; }

constexpr ColumnFormat BOOL = {std::ios::boolalpha, 3};

// --- Internal Table State ---
// ----------------------------

inline std::vector<Column> columns;
inline std::size_t         current_column = 0;
inline std::ostream*       output_stream  = &std::cout;
inline bool                latex_mode     = false;

// ===================
// --- Table Setup ---
// ===================

inline void create(std::initializer_list<uint>&& widths) {
    columns.resize(widths.size());
    for (std::size_t i = 0; i < columns.size(); ++i) {
        columns[i].width      = widths.begin()[i];
        columns[i].col_format = DEFAULT();
    }
}

inline void set_formats(std::initializer_list<ColumnFormat>&& formats) {
    for (std::size_t i = 0; i < columns.size(); ++i) columns[i].col_format = formats.begin()[i];
}

inline void set_ostream(std::ostream& new_ostream) { output_stream = &new_ostream; }

inline void set_latex_mode(bool toggle) { latex_mode = toggle; }

// =======================
// --- Table Rendering ---
// =======================

// We want to only apply additional typesetting to "actual mathematical numbers", not bools & chars
template <class T>
constexpr bool _is_arithmetic_number_v =
    std::is_arithmetic_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char>;

[[nodiscard]] inline std::string _trim_left(const std::string& str, char trimmed_char) {
    std::string res = str;
    res.erase(0, res.find_first_not_of(trimmed_char));
    return res;
}

// Function that adds some LaTeX decorators to appropriate types
template <class T>
void _append_decorated_value(std::ostream& os, const T& value) {
    using V = std::decay_t<T>;

    if (!latex_mode) {
        os << value;
        return;
    }

    if constexpr (_is_arithmetic_number_v<V>) {
        // In order to respect format flags of the table, we have to copy fmt into a stringstream
        // and use IT to stringify a number, simple 'std::to_string()' won't do it here
        std::ostringstream ss;
        ss.copyfmt(os);
        ss.width(0); // cancel out 'std::setw()' that was copied with all the other flags
        ss << value;
        std::string number_string = ss.str();

        // Convert scientific form number to a LaTeX-friendly form,
        // for example, "1.3e-15" becomes "1.3 \cdot 10^{-15}"
        const std::size_t e_index = number_string.find('e');
        if (e_index != std::string::npos) {
            const std::string mantissa = number_string.substr(0, e_index - 1);
            const char        sign     = number_string.at(e_index + 1);
            const std::string exponent = number_string.substr(e_index + 2);

            const bool mantissa_is_one =
                mantissa == "1" || mantissa == "1." || mantissa == "1.0" || mantissa == "1.00" || mantissa == "1.000";
            // dirty, simple, a sensible person would figure this out with math a lookup tables

            number_string.clear();
            if (!mantissa_is_one) { // powers of 10 don't need the fractional part
                number_string += mantissa;
                number_string += " \\cdot ";
            }
            number_string += "10^{";
            if (sign == '-') number_string += sign;
            const std::string trimmed_exponent = _trim_left(exponent, '0');
            number_string += trimmed_exponent.empty() ? "0" : trimmed_exponent; // prevent stuff like '10^{}'
            number_string += '}';
        }

        // Typeset numbers as formulas
        os << "$" + number_string + "$";
        // we append it as a single string so ostream 'setw()' doesn't mess up alignment
    } else os << value;
}

inline void cell(){};

template <class T, class... Types>
void cell(const T& value, const Types&... other_values) {
    const auto left_delim  = latex_mode ? "" : "|";
    const auto delim       = latex_mode ? " & " : "|";
    const auto right_delim = latex_mode ? " \\\\\n" : "|\n";

    const std::string left_cline      = (current_column == 0) ? left_delim : "";
    const std::string right_cline     = (current_column == columns.size() - 1) ? right_delim : delim;
    const ios_flags   format          = columns[current_column].col_format.flags;
    const uint        float_precision = columns[current_column].col_format.precision;

    // Save old stream state
    std::ios old_state(nullptr);
    old_state.copyfmt(*output_stream);

    // Set table formatting
    (*output_stream) << std::resetiosflags((*output_stream).flags());
    (*output_stream).flags(format);
    (*output_stream).precision(float_precision);

    // Print
    (*output_stream) << left_cline << std::setw(columns[current_column].width);
    _append_decorated_value(*output_stream, value);
    (*output_stream) << right_cline;

    // Return old stream state
    (*output_stream).copyfmt(old_state);

    // Advance column counter
    current_column = (current_column == columns.size() - 1) ? 0 : current_column + 1;

    cell(other_values...);
}

inline void hline() {
    if (latex_mode) {
        (*output_stream) << "\\hline\n";
    } else {
        (*output_stream) << "|";
        for (const auto& col : columns)
            (*output_stream) << std::string(static_cast<std::size_t>(col.width), '-') << "|";
        (*output_stream) << "\n";
    }
}

} // namespace utl::table::impl

// ______________________ PUBLIC API ______________________

namespace utl::table {

using impl::uint;

using impl::create;

using impl::set_formats;
using impl::set_ostream;
using impl::set_latex_mode;

using impl::cell;
using impl::hline;

using impl::NONE;
using impl::FIXED;
using impl::DEFAULT;
using impl::SCIENTIFIC;
using impl::BOOL;

} // namespace utl::table

#endif
#endif // module utl::table
