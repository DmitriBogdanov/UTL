// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::profiler
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_profiler.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_PROFILER)
#ifndef UTLHEADERGUARD_PROFILER
#define UTLHEADERGUARD_PROFILER

#define UTL_PROFILER_VERSION_MAJOR 1
#define UTL_PROFILER_VERSION_MINOR 0
#define UTL_PROFILER_VERSION_PATCH 0

// _______________________ INCLUDES _______________________

#ifndef UTL_PROFILER_DISABLE

#include <array>         // array<>, size_t
#include <cassert>       // assert()
#include <charconv>      // to_chars()
#include <chrono>        // steady_clock, duration<>
#include <cstdint>       // uint16_t, uint32_t
#include <iostream>      // cout
#include <mutex>         // mutex, lock_guard
#include <string>        // string, to_string()
#include <string_view>   // string_view
#include <thread>        // thread::id, this_thread::get_id()
#include <type_traits>   // enable_if_t<>, is_enum_v<>, is_invokable_v<>, underlying_type_t<>
#include <unordered_map> // unordered_map<>
#include <vector>        // vector<>

#endif // no need to pull all these headers with profiling disabled

// ____________________ DEVELOPER DOCS ____________________

// Optional macros:
// - #define UTL_PROFILER_DISABLE                            // disable all profiling
// - #define UTL_PROFILER_USE_INTRINSICS_FOR_FREQUENCY 3.3e9 // use low-overhead rdtsc timestamps
// - #define UTL_PROFILER_USE_SMALL_IDS                      // use 16-bit ids
//
// This used to be a much simpler header with a few macros to profile scope & print a flat table, it
// already applied the idea of using static variables to mark callsites efficiently and later underwent
// a full rewrite to add proper threading & call graph support.
//
// A lot of though went into making it fast, the key idea is to use 'thread_local' callsite
// markers to associate callsites with numeric thread-specific IDs and to reduce all call graph
// traversal to simple integer array lookups. Store everything we can densely, minimize locks,
// delay formatting and result evaluation as much as possible.
//
// Docs & comments scattered through code should explain the details decently well.

// ____________________ IMPLEMENTATION ____________________

#ifndef UTL_PROFILER_DISABLE
// '#ifndef' that wraps almost entire header,
// in '#else' branch only no-op mocks of the public API are compiled

// ==================================
// --- Optional __rdtsc() support ---
// ==================================

#ifdef UTL_PROFILER_USE_INTRINSICS_FOR_FREQUENCY

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#define utl_profiler_cpu_counter __rdtsc()

#endif

// ====================
// --- String utils ---
// ====================

namespace utl::profiler::impl {

constexpr std::size_t max(std::size_t a, std::size_t b) noexcept {
    return (a < b) ? b : a;
} // saves us a heavy <algorithm> include

template <class... Args>
void append_fold(std::string& str, const Args&... args) {
    ((str += args), ...);
} // faster than 'std::ostringstream' and saves us an include

inline std::string format_number(double value, std::chars_format format, int precision) {
    std::array<char, 30> buffer; // 80-bit 'long double' fits in 29, 64-bit 'double' in 24, this is always enough
    const auto end_ptr = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value, format, precision).ptr;
    return std::string(buffer.data(), end_ptr);
}

inline std::string format_call_site(std::string_view file, int line, std::string_view func) {
    const std::string_view filename = file.substr(file.find_last_of("/\\") + 1);

    std::string res;
    res.reserve(filename.size() + func.size() + 10); // +10 accounts for formatting chars and up to 5 line digits
    append_fold(res, filename, ":", std::to_string(line), ", ", func, "()");
    return res;
}

inline void append_aligned_right(std::string& str, const std::string& source, std::size_t width, char fill = ' ') {
    assert(width >= source.size());

    const std::size_t pad_left = width - source.size();
    str.append(pad_left, fill) += source;
}

inline void append_aligned_left(std::string& str, const std::string& source, std::size_t width, char fill = ' ') {
    assert(width >= source.size());

    const std::size_t pad_right = width - source.size();
    (str += source).append(pad_right, fill);
}

// ==============
// --- Timing ---
// ==============

// If we know CPU frequency at compile time we can wrap '__rdtsc()' into a <chrono>-compatible
// clock and use it seamlessly, no need for conditional compilation anywhere else

#ifdef UTL_PROFILER_USE_INTRINSICS_FOR_FREQUENCY
struct clock {
    using rep                   = unsigned long long int;
    using period                = std::ratio<1, static_cast<rep>(UTL_PROFILER_USE_INTRINSICS_FOR_FREQUENCY)>;
    using duration              = std::chrono::duration<rep, period>;
    using time_point            = std::chrono::time_point<clock>;
    static const bool is_steady = true;

    static time_point now() noexcept { return time_point(duration(utl_profiler_cpu_counter)); }
};
#else
using clock   = std::chrono::steady_clock;
#endif

using duration   = clock::duration;
using time_point = clock::time_point;

using ms = std::chrono::duration<double, std::chrono::milliseconds::period>;
// float time makes conversions more convenient

// =====================
// --- Type-safe IDs ---
// =====================

template <class Enum, std::enable_if_t<std::is_enum_v<Enum>, bool> = true>
[[nodiscard]] constexpr auto to_int(Enum value) noexcept {
    return static_cast<std::underlying_type_t<Enum>>(value);
}

#ifdef UTL_PROFILER_USE_SMALL_IDS
using id_type = std::uint16_t;
#else
using id_type = std::uint32_t;
#endif

enum class CallsiteId : id_type { empty = to_int(CallsiteId(-1)) };
enum class NodeId : id_type { root = 0, empty = to_int(NodeId(-1)) };

struct CallsiteInfo {
    const char* file;
    const char* func;
    const char* label;
    int         line;
    // 'file', 'func', 'label' are guaranteed to be string literals, since we want to
    // have as little overhead as possible during runtime, we can just save raw pointers
    // and convert them to nicer types like 'std::string_view' later in the formatting stage
};

// ==================
// --- Formatting ---
// ==================

struct Style {
    std::size_t indent = 2;
    bool        color  = true;

    double cutoff_red    = 0.40; // > 40% of total runtime
    double cutoff_yellow = 0.20; // > 20% of total runtime
    double cutoff_gray   = 0.01; // <  1% of total runtime
};

namespace color {

constexpr std::string_view red          = "\033[31m";   // call graph rows that take very significant time
constexpr std::string_view yellow       = "\033[33m";   // call graph rows that take significant time
constexpr std::string_view gray         = "\033[90m";   // call graph rows that take very little time
constexpr std::string_view bold_cyan    = "\033[36;1m"; // call graph headings
constexpr std::string_view bold_green   = "\033[32;1m"; // joined  threads
constexpr std::string_view bold_magenta = "\033[35;1m"; // running threads
constexpr std::string_view bold_blue    = "\033[34;1m"; // thread runtime

constexpr std::string_view reset = "\033[0m";

} // namespace color

struct FormattedRow {
    CallsiteInfo callsite;
    duration     time;
    std::size_t  depth;
    double       percentage;
};

// =================================
// --- Call graph core structure ---
// =================================

class NodeMatrix {
    template <class T>
    using array_type = std::vector<T>;
    // Note: Using 'std::unique_ptr<T[]> arrays would shave off 64 bytes from 'sizeof(NodeMatrix)',
    //       but it's cumbersome and not particularly important for performance

    constexpr static std::size_t col_growth_mul = 2;
    constexpr static std::size_t row_growth_add = 4;
    // - rows capacity grows additively in fixed increments
    // - cols capacity grows multiplicatively
    // this unusual growth strategy is due to our anticipated growth pattern - callsites are few, every
    // one needs to be manually created by the user, nodes can quickly grow in number due to recursion

    array_type<NodeId> prev_ids;
    // [ nodes ] dense vector encoding backwards-traversal of a call graph
    // 'prev_ids[node_id]' -> id of the previous node in the call graph for 'node_id'

    array_type<NodeId> next_ids;
    // [ callsites x nodes ] dense matrix encoding forward-traversal of a call graph
    // 'next_ids(callsite_id, node_id)' -> id of the next node in the call graph for 'node_id' at 'callsite_id',
    //                                     storage is col-major due to our access pattern

    // Note: Both 'prev_ids' and 'next_ids' will contain 'NodeId::empty' values at positions with no link

    array_type<duration> times;
    // [ nodes ] dense vector containing time spent at each node of the call graph
    // 'times[node_id]' -> total time spent at 'node_id'

    array_type<CallsiteInfo> callsites;
    // [ callsites ] dense vector containing info about the callsites
    // 'callsites[callsite_id]' -> pointers to file/function/label & line

    std::size_t rows_size     = 0;
    std::size_t cols_size     = 0;
    std::size_t rows_capacity = 0;
    std::size_t cols_capacity = 0;

public:
    std::size_t rows() const noexcept { return this->rows_size; }
    std::size_t cols() const noexcept { return this->cols_size; }

    bool empty() const noexcept { return this->rows() == 0 || this->cols() == 0; }

    // - Access (mutable) -

    NodeId& prev_id(NodeId node_id) {
        assert(to_int(node_id) < this->cols());
        return this->prev_ids[to_int(node_id)];
    }

    NodeId& next_id(CallsiteId callsite_id, NodeId node_id) {
        assert(to_int(callsite_id) < this->rows());
        assert(to_int(node_id) < this->cols());
        return this->next_ids[to_int(callsite_id) + to_int(node_id) * this->rows_capacity];
    }

    duration& time(NodeId node_id) {
        assert(to_int(node_id) < this->cols());
        return this->times[to_int(node_id)];
    }

    CallsiteInfo& callsite(CallsiteId callsite_id) {
        assert(to_int(callsite_id) < this->rows());
        return this->callsites[to_int(callsite_id)];
    }

    // - Access (const) -

    const NodeId& prev_id(NodeId node_id) const {
        assert(to_int(node_id) < this->cols());
        return this->prev_ids[to_int(node_id)];
    }

    const NodeId& next_id(CallsiteId callsite_id, NodeId node_id) const {
        assert(to_int(callsite_id) < this->rows());
        assert(to_int(node_id) < this->cols());
        return this->next_ids[to_int(callsite_id) + to_int(node_id) * this->rows_capacity];
    }

    const duration& time(NodeId node_id) const {
        assert(to_int(node_id) < this->cols());
        return this->times[to_int(node_id)];
    }

    const CallsiteInfo& callsite(CallsiteId callsite_id) const {
        assert(to_int(callsite_id) < this->rows());
        return this->callsites[to_int(callsite_id)];
    }

    // - Resizing -

    void resize(std::size_t new_rows, std::size_t new_cols) {
        const bool new_rows_over_capacity = new_rows > this->rows_capacity;
        const bool new_cols_over_capacity = new_cols > this->cols_capacity;
        const bool requires_reallocation  = new_rows_over_capacity || new_cols_over_capacity;

        // No reallocation case
        if (!requires_reallocation) {
            this->rows_size = new_rows;
            this->cols_size = new_cols;
            return;
        }

        // Reallocate
        const std::size_t new_rows_capacity =
            new_rows_over_capacity ? new_rows + NodeMatrix::row_growth_add : this->rows_capacity;
        const std::size_t new_cols_capacity =
            new_cols_over_capacity ? new_cols * NodeMatrix::col_growth_mul : this->cols_capacity;

        array_type<NodeId>       new_prev_ids(new_cols_capacity, NodeId::empty);
        array_type<NodeId>       new_next_ids(new_rows_capacity * new_cols_capacity, NodeId::empty);
        array_type<duration>     new_times(new_cols_capacity, duration{});
        array_type<CallsiteInfo> new_callsites(new_rows_capacity, CallsiteInfo{});

        // Copy old data
        for (std::size_t j = 0; j < this->cols_size; ++j) new_prev_ids[j] = this->prev_ids[j];
        for (std::size_t j = 0; j < this->cols_size; ++j)
            for (std::size_t i = 0; i < this->rows_size; ++i)
                new_next_ids[i + j * new_rows_capacity] = this->next_ids[i + j * this->rows_capacity];
        for (std::size_t j = 0; j < this->cols_size; ++j) new_times[j] = this->times[j];
        for (std::size_t i = 0; i < this->rows_size; ++i) new_callsites[i] = this->callsites[i];

        // Assign new data
        this->prev_ids  = std::move(new_prev_ids);
        this->next_ids  = std::move(new_next_ids);
        this->times     = std::move(new_times);
        this->callsites = std::move(new_callsites);

        this->rows_size     = new_rows;
        this->cols_size     = new_cols;
        this->rows_capacity = new_rows_capacity;
        this->cols_capacity = new_cols_capacity;
    }

    void grow_callsites() { this->resize(this->rows_size + 1, this->cols_size); }

    void grow_nodes() { this->resize(this->rows_size, this->cols_size + 1); }

    template <class Func, std::enable_if_t<std::is_invocable_v<Func, CallsiteId, NodeId, std::size_t>, bool> = true>
    void node_apply_recursively(CallsiteId callsite_id, NodeId node_id, Func func, std::size_t depth) const {
        func(callsite_id, node_id, depth);

        for (std::size_t i = 0; i < this->rows(); ++i) {
            const CallsiteId next_callsite_id = CallsiteId(i);
            const NodeId     next_node_id     = this->next_id(next_callsite_id, node_id);
            if (next_node_id != NodeId::empty)
                this->node_apply_recursively(next_callsite_id, next_node_id, func, depth + 1);
        }
        // 'node_is' corresponds to a matrix column, to iterate over all
        // "next" nodes we iterate rows (callsites) in a column
    }

    template <class Func, std::enable_if_t<std::is_invocable_v<Func, CallsiteId, NodeId, std::size_t>, bool> = true>
    void root_apply_recursively(Func func) const {
        if (!this->rows_size || !this->cols_size) return; // possibly redundant

        func(CallsiteId::empty, NodeId::root, 0);

        for (std::size_t i = 0; i < this->rows(); ++i) {
            const CallsiteId next_callsite_id = CallsiteId(i);
            const NodeId     next_node_id     = this->next_id(next_callsite_id, NodeId::root);
            if (next_node_id != NodeId::empty) this->node_apply_recursively(next_callsite_id, next_node_id, func, 1);
        }
    }
};

// ================
// --- Profiler ---
// ================

struct ThreadLifetimeData {
    NodeMatrix mat;
    bool       joined = false;
};

struct ThreadIdData {
    std::vector<ThreadLifetimeData> lifetimes;
    std::size_t                     readable_id;
    // since we need a map from 'std::thread::id' to both lifetimes and human-readable id mappings,
    // and those maps would be accessed at the same time, it makes sense to instead avoid a second
    // map lookup and merge both values into a single struct
};

class Profiler {
    // header-inline, only one instance exists, this instance is effectively a persistent
    // "database" responsible for collecting & formatting results

    using call_graph_storage = std::unordered_map<std::thread::id, ThreadIdData>;
    // thread ID by itself is not enough to identify a distinct thread with a finite lifetime, OS only guarantees
    // unique thread ids for currently existing threads, new threads may reuse IDs of the old joined threads,
    // this is why for every thread ID we store a vector - this vector grows every time a new thread with a given
    // id is created

    friend struct ThreadCallGraph;

    call_graph_storage call_graph_info;
    std::mutex         call_graph_mutex;

    std::thread::id main_thread_id = std::this_thread::get_id();
    std::size_t     thread_counter = 0;

    bool       print_at_destruction = true;
    std::mutex setter_mutex;

    bool results_are_empty() {
        // A check used to deduce whether we have any meaningful results to automatically print after exit,
        // all threads joined, yet none of them contain any profiling records <=> no profiling was ever invoked
        for (const auto& [thread_id, thread_lifetimes] : this->call_graph_info)
            for (const auto& lifetime : thread_lifetimes.lifetimes)
                if (lifetime.joined == false || !lifetime.mat.empty()) return false;

        return true;
    }

    std::string format_available_results(const Style& style = Style{}) {
        const std::lock_guard lock(this->call_graph_mutex);

        std::vector<FormattedRow> rows;
        std::string               res;

        // Format header
        if (style.color) res += color::bold_cyan;
        append_fold(res, "\n-------------------- UTL PROFILING RESULTS ---------------------\n");
        if (style.color) res += color::reset;

        for (const auto& [thread_id, thread_lifetimes] : this->call_graph_info) {
            for (std::size_t reuse = 0; reuse < thread_lifetimes.lifetimes.size(); ++reuse) {
                const auto&       mat         = thread_lifetimes.lifetimes[reuse].mat;
                const bool        joined      = thread_lifetimes.lifetimes[reuse].joined;
                const std::size_t readable_id = thread_lifetimes.readable_id;

                rows.clear();
                rows.reserve(mat.cols());

                const std::string thread_str      = (readable_id == 0) ? "main" : std::to_string(readable_id);
                const bool        thread_uploaded = !mat.empty();

                // Format thread header
                if (style.color) res += color::bold_cyan;
                append_fold(res, "\n# Thread [", thread_str, "] (reuse ", std::to_string(reuse), ")");
                if (style.color) res += color::reset;

                // Format thread status
                if (style.color) res += joined ? color::bold_green : color::bold_magenta;
                append_fold(res, joined ? " (joined)" : " (running)");
                if (style.color) res += color::reset;

                // Early escape for lifetimes that haven't uploaded yet
                if (!thread_uploaded) {
                    append_fold(res, '\n');
                    continue;
                }

                // Format thread runtime
                const ms   runtime     = mat.time(NodeId::root);
                const auto runtime_str = format_number(runtime.count(), std::chars_format::fixed, 2);

                if (style.color) res += color::bold_blue;
                append_fold(res, " (runtime -> ", runtime_str, " ms)\n");
                if (style.color) res += color::reset;

                // Gather call graph data in a digestible format
                mat.root_apply_recursively([&](CallsiteId callsite_id, NodeId node_id, std::size_t depth) {
                    if (callsite_id == CallsiteId::empty) return;

                    const auto&  callsite   = mat.callsite(callsite_id);
                    const auto&  time       = mat.time(node_id);
                    const double percentage = time / runtime;

                    rows.push_back(FormattedRow{callsite, time, depth, percentage});
                });

                // Format call graph columns row by row
                std::vector<std::array<std::string, 4>> rows_str;
                rows_str.reserve(rows.size());

                for (const auto& row : rows) {
                    const auto percentage_num_str = format_number(row.percentage * 100, std::chars_format::fixed, 2);

                    auto percentage_str = std::string(style.indent * row.depth, ' ');
                    append_fold(percentage_str, " - ", percentage_num_str, "% ");

                    auto time_str     = format_number(ms(row.time).count(), std::chars_format::fixed, 2) + " ms";
                    auto label_str    = std::string(row.callsite.label);
                    auto callsite_str = format_call_site(row.callsite.file, row.callsite.line, row.callsite.func);

                    rows_str.push_back({std::move(percentage_str), std::move(time_str), std::move(label_str),
                                        std::move(callsite_str)});
                }

                // Gather column widths for alignment
                std::size_t width_percentage = 0, width_time = 0, width_label = 0, width_callsite = 0;
                for (const auto& row : rows_str) {
                    width_percentage = max(width_percentage, row[0].size());
                    width_time       = max(width_time, row[1].size());
                    width_label      = max(width_label, row[2].size());
                    width_callsite   = max(width_callsite, row[3].size());
                }

                assert(rows.size() == rows_str.size());

                // Format resulting string with colors & alignment
                for (std::size_t i = 0; i < rows.size(); ++i) {
                    const bool color_row_red     = style.color && rows[i].percentage > style.cutoff_red;
                    const bool color_row_yellow  = style.color && rows[i].percentage > style.cutoff_yellow;
                    const bool color_row_gray    = style.color && rows[i].percentage < style.cutoff_gray;
                    const bool color_was_applied = color_row_red || color_row_yellow || color_row_gray;

                    if (color_row_red) res += color::red;
                    else if (color_row_yellow) res += color::yellow;
                    else if (color_row_gray) res += color::gray;

                    append_aligned_left(res, rows_str[i][0], width_percentage, '-');
                    append_fold(res, " | ");
                    append_aligned_right(res, rows_str[i][1], width_time);
                    append_fold(res, " | ");
                    append_aligned_right(res, rows_str[i][2], width_label);
                    append_fold(res, " | ");
                    append_aligned_left(res, rows_str[i][3], width_callsite);
                    append_fold(res, " |");

                    if (color_was_applied) res += color::reset;

                    res += '\n';
                }
            }
        }

        return res;
    }

    void call_graph_add(std::thread::id thread_id) {
        const std::lock_guard lock(this->call_graph_mutex);

        const auto [it, emplaced] = this->call_graph_info.try_emplace(thread_id);

        // Emplacement took place =>
        // This is the first time inserting this thread id, add human-readable mapping that grows by 1 for each new
        // thread, additional checks are here to ensure that main thread is always '0' and other threads are always
        // '1+' even if main thread wasn't the first one to register a profiler. This is important because formatting
        // prints zero-thread as '[main]' and we don't want this title to go to some other thread
        if (emplaced) it->second.readable_id = (thread_id == this->main_thread_id) ? 0 : ++this->thread_counter;

        // Add a default-constructed call graph matrix to lifetimes,
        // - if this thread ID was emplaced      then this is a non-reused thread ID
        // - if this thread ID was already there then this is a     reused thread ID
        // regardless, our actions are the same
        it->second.lifetimes.emplace_back();
    }

    void call_graph_upload(std::thread::id thread_id, NodeMatrix&& info, bool joined) {
        const std::lock_guard lock(this->call_graph_mutex);

        auto& lifetime  = this->call_graph_info.at(thread_id).lifetimes.back();
        lifetime.mat    = std::move(info);
        lifetime.joined = joined;
    }

public:
    void upload_this_thread(); // depends on the 'ThreadCallGraph', defined later

    void print_at_exit(bool value) noexcept {
        const std::lock_guard lock(this->setter_mutex);
        // useless most of the time, but allows public API to be completely thread-safe

        this->print_at_destruction = value;
    }

    std::string format_results(const Style& style = Style{}) {
        this->upload_this_thread();
        // Call graph from current thread is not yet uploaded by its 'thread_local' destructor, we need to
        // explicitly pull it which we can easily do since current thread can't contest its own resources

        return this->format_available_results(style);
    }

    ~Profiler() {
        if (!this->print_at_destruction) return; // printing was manually disabled
        if (this->results_are_empty()) return;   // no profiling was ever invoked
        std::cout << format_available_results();
    }
};

inline Profiler profiler;

// =========================
// --- Thread Call Graph ---
// =========================

struct ThreadCallGraph {
    // header-inline-thread_local, gets created whenever we create a new thread anywhere,
    // this class is responsible for managing some thread-specific things on top of our
    // core graph traversal structure and provides an actual high-level API for graph traversal

    NodeMatrix      mat;
    NodeId          current_node_id  = NodeId::empty;
    time_point      entry_time_point = clock::now();
    std::thread::id thread_id        = std::this_thread::get_id();

    NodeId create_root_node() {
        const NodeId prev_node_id = this->current_node_id;
        this->current_node_id     = NodeId::root; // advance to a new node

        this->mat.grow_nodes();
        this->mat.prev_id(this->current_node_id) = prev_node_id;
        // link new node backwards, since this is a root the prev. one is empty and doesn't need to link forwards

        return this->current_node_id;
    }

    NodeId create_node(CallsiteId callsite_id) {
        const NodeId prev_node_id = this->current_node_id;
        this->current_node_id     = NodeId(this->mat.cols()); // advance to a new node

        this->mat.grow_nodes();
        this->mat.prev_id(this->current_node_id)     = prev_node_id;          // link new node backwards
        this->mat.next_id(callsite_id, prev_node_id) = this->current_node_id; // link prev. node forwards

        return this->current_node_id;
    }

    void upload_results(bool joined) {
        this->mat.time(NodeId::root) = clock::now() - this->entry_time_point;
        // root node doesn't get time updates from timers, we need to collect total runtime manually

        profiler.call_graph_upload(this->thread_id, NodeMatrix(this->mat), joined); // deep copy & mutex lock, slow
    }

public:
    ThreadCallGraph() {
        profiler.call_graph_add(this->thread_id);

        this->create_root_node();
    }

    ~ThreadCallGraph() { this->upload_results(true); }

    NodeId traverse_forward(CallsiteId callsite_id) {
        const NodeId next_node_id = this->mat.next_id(callsite_id, this->current_node_id);
        // 1 dense matrix lookup to advance the node forward, 1 branch to check its existence
        // 'callsite_id' is always valid due to callsite & timer initialization order

        // - node missing  => create new node and return its id
        if (next_node_id == NodeId::empty) return this->create_node(callsite_id);

        // - node exists   =>  return existing id
        return this->current_node_id = next_node_id;
    }

    void traverse_back() { this->current_node_id = this->mat.prev_id(this->current_node_id); }

    void record_time(duration time) { this->mat.time(this->current_node_id) += time; }

    CallsiteId callsite_add(const CallsiteInfo& info) { // adds new callsite & returns its id
        const CallsiteId new_callsite_id = CallsiteId(this->mat.rows());

        this->mat.grow_callsites();
        this->mat.callsite(new_callsite_id) = info;

        return new_callsite_id;
    }
};

inline thread_local ThreadCallGraph thread_call_graph;

inline void Profiler::upload_this_thread() { thread_call_graph.upload_results(false); }

// =======================
// --- Callsite Marker ---
// =======================

struct Callsite {
    // local-thread_local, small marker binding a numeric ID to a callsite

    CallsiteId callsite_id;

public:
    Callsite(const CallsiteInfo& info) { this->callsite_id = thread_call_graph.callsite_add(info); }

    CallsiteId get_id() const noexcept { return this->callsite_id; }
};

// =============
// --- Timer ---
// =============

class Timer {
    time_point entry = clock::now();

public:
    Timer(CallsiteId callsite_id) { thread_call_graph.traverse_forward(callsite_id); }

    void finish() const {
        thread_call_graph.record_time(clock::now() - this->entry);
        thread_call_graph.traverse_back();
    }
};

struct ScopeTimer : public Timer { // just like regular timer, but finishes at the end of the scope
    ScopeTimer(CallsiteId callsite_id) : Timer(callsite_id) {}

    constexpr operator bool() const noexcept { return true; }
    // allows us to use create scope timers inside 'if constexpr' & have applies-to-next-expression semantics for macro

    ~ScopeTimer() { this->finish(); }
};

} // namespace utl::profiler::impl

// =====================
// --- Helper macros ---
// =====================

#define utl_profiler_concat_tokens(a, b) a##b
#define utl_profiler_concat_tokens_wrapper(a, b) utl_profiler_concat_tokens(a, b)
#define utl_profiler_uuid(varname_) utl_profiler_concat_tokens_wrapper(varname_, __LINE__)
// creates token 'varname_##__LINE__' from 'varname_', necessary
// to work around some macro expansion order shenanigans

// ______________________ PUBLIC API ______________________

// ==========================================
// --- Definitions with profiling enabled ---
// ==========================================

namespace utl::profiler {

using impl::Profiler;
using impl::profiler;
using impl::Style;

} // namespace utl::profiler

#define UTL_PROFILER_SCOPE(label_)                                                                                     \
    constexpr bool utl_profiler_uuid(utl_profiler_macro_guard_) = true;                                                \
    static_assert(utl_profiler_uuid(utl_profiler_macro_guard_), "UTL_PROFILER is a multi-line macro.");                \
                                                                                                                       \
    const thread_local utl::profiler::impl::Callsite utl_profiler_uuid(utl_profiler_callsite_)(                        \
        utl::profiler::impl::CallsiteInfo{__FILE__, __func__, label_, __LINE__});                                      \
                                                                                                                       \
    const utl::profiler::impl::ScopeTimer utl_profiler_uuid(utl_profiler_scope_timer_) {                               \
        utl_profiler_uuid(utl_profiler_callsite_).get_id()                                                             \
    }

// all variable names are concatenated with a line number to prevent shadowing when then there are multiple nested
// profilers, this isn't a 100% foolproof solution, but it works reasonably well. Shadowed variables don't have any
// effect on functionality, but might cause warnings from some static analysis tools
//
// 'constexpr bool' and 'static_assert()' are here to improve error messages when this macro is misused as an
// expression, when someone writes 'if (...) UTL_PROFILER_SCOPE(...) func()' instead of many ugly errors they
// will see a macro expansion that contains a 'static_assert()' with a proper message

#define UTL_PROFILER(label_)                                                                                           \
    constexpr bool utl_profiler_uuid(utl_profiler_macro_guard_) = true;                                                \
    static_assert(utl_profiler_uuid(utl_profiler_macro_guard_), "UTL_PROFILER is a multi-line macro.");                \
                                                                                                                       \
    const thread_local utl::profiler::impl::Callsite utl_profiler_uuid(utl_profiler_callsite_)(                        \
        utl::profiler::impl::CallsiteInfo{__FILE__, __func__, label_, __LINE__});                                      \
                                                                                                                       \
    if constexpr (const utl::profiler::impl::ScopeTimer utl_profiler_uuid(utl_profiler_scope_timer_){                  \
                      utl_profiler_uuid(utl_profiler_callsite_).get_id()})

// 'if constexpr (timer)' allows this macro to "capture" the scope of the following expression

#define UTL_PROFILER_BEGIN(segment_, label_)                                                                           \
    const thread_local utl::profiler::impl::Callsite utl_profiler_callsite_##segment_(                                 \
        utl::profiler::impl::CallsiteInfo{__FILE__, __func__, label_, __LINE__});                                      \
                                                                                                                       \
    const utl::profiler::impl::Timer utl_profiler_timer_##segment_ { utl_profiler_callsite_##segment_.get_id() }

#define UTL_PROFILER_END(segment_) utl_profiler_timer_##segment_.finish()

// ===========================================
// --- Definitions with profiling disabled ---
// ===========================================

// No-op mocks of the public API and minimal necessary includes

#else

#include <cstddef> // size_t
#include <string>  // string

namespace utl::profiler {
struct Style {
    std::size_t indent = 2;
    bool        color  = true;

    double cutoff_red    = 0.40;
    double cutoff_yellow = 0.20;
    double cutoff_gray   = 0.01;
};

struct Profiler {
    void print_at_exit(bool) noexcept {}

    void upload_this_thread() {}

    std::string format_results(const Style = Style{}) { return "<profiling is disabled>"; }
};
} // namespace utl::profiler

#define UTL_PROFILER_SCOPE(label_) static_assert(true)
#define UTL_PROFILER(label_)
#define UTL_PROFILER_BEGIN(segment_, label_) static_assert(true)
#define UTL_PROFILER_END(segment_) static_assert(true)

// 'static_assert(true)' emulates the "semicolon after the macro" requirement

#endif

#endif
#endif // module utl::profiler
