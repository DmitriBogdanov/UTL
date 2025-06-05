// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::progressbar
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_progressbar.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_PROGRESSBAR)
#ifndef UTLHEADERGUARD_PROGRESSBAR
#define UTLHEADERGUARD_PROGRESSBAR

#define UTL_PROGRESSBAR_VERSION_MAJOR 1
#define UTL_PROGRESSBAR_VERSION_MINOR 0
#define UTL_PROGRESSBAR_VERSION_PATCH 0

// _______________________ INCLUDES _______________________

#include <algorithm>   // max(), clamp()
#include <array>       // array
#include <charconv>    // to_chars
#include <chrono>      // chrono::steady_clock, chrono::time_point<>, chrono::duration_cast<>
#include <cstddef>     // size_t
#include <iostream>    // cout
#include <iterator>    // ostream_iterator<>
#include <string>      // string
#include <string_view> // string_view

// ____________________ DEVELOPER DOCS ____________________

// Simple progress bars for terminal applications. Rendered in ASCII on the main thread with manual updates
// for maximal compatibility. Perhaps can be extended with some fancier async options that display animations.
//
// Used to be implemented in terms of 'std::stringstream' but later got rewritten to improve performance,
// reduce includes, allow more style configuration and make API more robust against misuse.

// ____________________ IMPLEMENTATION ____________________

namespace utl::progressbar::impl {

// Proper progress bar, uses '\r' to render new state in the same spot.
// Allocates when formatting things for the first time, after that storage gets reused.
class Percentage {
public:
    // - Public parameters -
    struct Style {
        char        fill            = '#';
        char        empty           = '.';
        char        left            = '[';
        char        right           = ']';
        std::string estimate_prefix = "(remaining: ";
        std::string estimate_suffix = ")";
    } style;

    bool show_bar        = true;
    bool show_percentage = true;
    bool show_estimate   = true;

    std::size_t bar_length  = 30;
    double      update_rate = 2.5e-3; // every quarter of a % feels like a good default

    // - Public API -
    Percentage() : start_time_point(clock::now()) {
        std::cout << '\n';
        this->draw();
        std::cout.flush();
    }

    void set_progress(double value) {
        value = std::clamp(value, 0., 1.);

        if (value - this->progress < this->update_rate) return; // prevents progress decrement

        this->progress = value;

        this->draw();
        std::cout.flush();
    }

    void finish() {
        if (this->finished) return; // prevents weird formatting from multiple 'finish()' calls

        this->progress = 1.;
        this->finished = true;

        this->draw();
        std::cout << '\n';
        std::cout.flush();
    }

    void update_style() {
        this->draw();
        std::cout.flush();
    }

private:
    // - Internal state -
    using clock = std::chrono::steady_clock;

    clock::time_point start_time_point = clock::now();
    std::size_t       max_drawn_length = 0;
    double            progress         = 0;
    bool              finished         = false;

    std::string buffer; // keep the buffer so we don't have to reallocate each time

    void format_bar() {
        if (!this->show_bar) return;

        const std::size_t fill_length  = static_cast<std::size_t>(this->progress * this->bar_length);
        const std::size_t empty_length = this->bar_length - fill_length;

        this->buffer += this->style.left;
        this->buffer.append(fill_length, this->style.fill);
        this->buffer.append(empty_length, this->style.empty);
        this->buffer += this->style.right;
        this->buffer += ' ';
    }

    void format_percentage() {
        if (!this->show_percentage) return;

        constexpr auto        format    = std::chars_format::fixed;
        constexpr std::size_t precision = 2;
        constexpr std::size_t max_chars = 6; // enough for for '0.xx' to '100.xx',

        std::array<char, max_chars> chars;
        const double                percentage = this->progress * 100; // 'set_progress()' enforces 0 <= progress <= 1

        const auto end_ptr = std::to_chars(chars.data(), chars.data() + max_chars, percentage, format, precision).ptr;
        // can't error, buffer size is guaranteed to be enough

        this->buffer.append(chars.data(), end_ptr - chars.data());
        this->buffer += '%';
        this->buffer += ' ';
    }

    void format_estimate() {
        if (!this->show_estimate) return;
        if (!this->progress) return;

        const auto elapsed  = clock::now() - this->start_time_point;
        const auto estimate = elapsed * (1. - this->progress) / this->progress;

        const auto hours   = std::chrono::duration_cast<std::chrono::hours>(estimate);
        const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(estimate - hours);
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(estimate - hours - minutes);

        this->buffer += this->style.estimate_prefix;

        if (hours.count()) {
            this->buffer += std::to_string(hours.count());
            this->buffer += " hours ";
            this->buffer += std::to_string(minutes.count());
            this->buffer += " min ";
            this->buffer += std::to_string(seconds.count());
            this->buffer += " sec";
        } else if (minutes.count()) {
            this->buffer += std::to_string(minutes.count());
            this->buffer += " min ";
            this->buffer += std::to_string(seconds.count());
            this->buffer += " sec";
        } else {
            this->buffer += std::to_string(seconds.count());
            this->buffer += " sec";
        }

        this->buffer += this->style.estimate_suffix;
    }

    void draw() {
        this->buffer.clear();
        this->buffer += '\r';

        // Draw progressbar
        this->format_bar();
        this->format_percentage();
        this->format_estimate();

        // Draw spaces over potential remains of the previous bar (which could be longer due to time estimate)
        this->max_drawn_length = std::max(this->max_drawn_length, this->buffer.size());
        this->buffer.append(this->max_drawn_length - this->buffer.size(), ' ');

        std::cout << this->buffer;
    }
};

// Minimalistic progress bar, used when terminal doesn't support '\r' (they exist).
// Does not allocate.
class Ruler {
    constexpr static std::string_view ticks      = "0    10   20   30   40   50   60   70   80   90   100%";
    constexpr static std::string_view ruler      = "|----|----|----|----|----|----|----|----|----|----|";
    constexpr static std::size_t      bar_length = ruler.size();

public:
    // - Public parameters -
    struct Style {
        char fill            = '#';
        char ruler_line      = '-';
        char ruler_delimiter = '|';
    } style;

    bool show_ticks = true;
    bool show_ruler = true;
    bool show_bar   = true; // useless, but might as well have it for uniformity

    // - Public API -
    Ruler() {
        std::cout << '\n';
        this->draw_ticks();
        std::cout << '\n';
        this->draw_ruler();
        std::cout << '\n';
        std::cout.flush();
    }

    void set_progress(double value) {
        value = std::clamp(value, 0., 1.);

        this->progress_in_chars = static_cast<std::size_t>(this->bar_length * value);

        this->draw_bar();
        std::cout.flush();
    }

    void finish() {
        if (this->finished) return; // prevents weird formatting from multiple 'finish()' calls

        this->progress_in_chars = this->bar_length;
        this->finished          = true;

        this->draw_bar();
        std::cout << '\n';
        std::cout.flush();
    }

private:
    // - Internal state -
    std::size_t progress_in_chars = 0;
    std::size_t chars_drawn       = 0;
    bool        finished          = false;

    void draw_ticks() {
        if (!this->show_ticks) return;
        std::cout << this->ticks;
    }

    void draw_ruler() {
        if (!this->show_ruler) return;

        std::array<char, ruler.size()> buffer;
        for (std::size_t i = 0; i < ruler.size(); ++i)
            buffer[i] = (this->ruler[i] == '|') ? this->style.ruler_delimiter : this->style.ruler_line;
        // formats ruler without allocating

        std::cout.write(buffer.data(), buffer.size());
    }

    void draw_bar() {
        if (!this->show_bar) return;

        if (this->progress_in_chars > this->chars_drawn)
            std::fill_n(std::ostream_iterator<char>(std::cout), this->progress_in_chars - this->chars_drawn,
                        this->style.fill);

        this->chars_drawn = this->progress_in_chars;
    }
};

} // namespace utl::progressbar::impl

// ______________________ PUBLIC API ______________________

namespace utl::progressbar {

using impl::Percentage;
using impl::Ruler;

} // namespace utl::progressbar

#endif
#endif // module utl::progressbar
