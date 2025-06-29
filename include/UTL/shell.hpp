// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::shell
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_shell.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_SHELL)
#ifndef UTLHEADERGUARD_SHELL
#define UTLHEADERGUARD_SHELL

#define UTL_SHELL_VERSION_MAJOR 1
#define UTL_SHELL_VERSION_MINOR 0
#define UTL_SHELL_VERSION_PATCH 1

// _______________________ INCLUDES _______________________

#include <cstdlib>     // system()
#include <filesystem>  // fs::remove(), fs::path, fs::exists(), fs::temp_directory_path()
#include <fstream>     // ofstream, ifstream
#include <stdexcept>   // runtime_error
#include <string>      // string, size_t
#include <string_view> // string_view
#include <thread>      // thread::id, this_thread::get_id()

// ____________________ DEVELOPER DOCS ____________________

// RAII handles for temporary file creation, 'std::system()' wrapper to execute shell commands.
//
// Running commands while capturing both stdout & stderr is surprisingly difficult to do in standard
// C++, piping output to temporary files and then reading them seems to be the most portable way since
// piping works the same way in the vast majority of shells including Windows 'batch'.
//
// API tries to be robust, but at the end of the day we're largely at the whim of the system when it comes
// to filesystem races and shell execution, not much that we could do without hooking to system APIs directly.

// ____________________ IMPLEMENTATION ____________________

namespace utl::shell::impl {

// ==================================
// --- Random filename generation ---
// ==================================

// To generate random filenames we need to generate random characters

// For our use case entropy source has 3 requirements:
//    1. It should be different on each run
//    2. It should be different on each thread
//    3. It should be thread-safe
// Good statistical quality isn't particularly important.
std::uint64_t entropy() {
    const std::uint64_t time_entropy   = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const std::uint64_t thread_entropy = std::hash<std::thread::id>{}(std::this_thread::get_id());
    return time_entropy ^ thread_entropy;
};

// We want to avoid modifying the state of 'std::rand()', so we use a fast & small thread-local PRNG
constexpr std::uint64_t splitmix64(std::uint64_t state) noexcept {
    std::uint64_t result = (state += 0x9E3779B97f4A7C15);
    result               = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
    result               = (result ^ (result >> 27)) * 0x94D049BB133111EB;
    return result ^ (result >> 31);
}

// Since char distribution quality isn't particularly important, we can avoid
// including the whole <random> and just use a biased remainder formula
inline char random_char() {
    thread_local std::uint64_t state = entropy();

    state = splitmix64(state);

    constexpr char min_char = 'a';
    constexpr char max_char = 'z';

    return static_cast<char>(min_char + state % std::uint64_t(max_char - min_char + 1));
}

// In the end we have a pretty fast general-purpose random string function
inline std::string random_ascii_string(std::size_t length = 20) {
    std::string result(length, '0');
    for (std::size_t i = 0; i < result.size(); ++i) result[i] = random_char();
    return result;
}

// =======================
// --- Temporary files ---
// =======================

// RAII handle to a temporary file, based on the filesystem and
// naming rather than a proper file handle due to following reasons:
//
//    1. Some temp. file uses (such as command piping) require file to be closed and then reopened again,
//       which means we can't have a persistent file handle without sacrificing a major use case
//
//    2. Before C++23 there is no portable way to open file in exclusive mode,
//       which means we will have possible filesystem race regardless of API
//
struct TemporaryHandle {
private:
    std::filesystem::path filepath;
    std::string           string;
    // makes sense to cache the path string, considering that it is immutable and frequently needed

    explicit TemporaryHandle(std::filesystem::path&& filepath)
        : filepath(std::move(filepath)), string(this->filepath.string()) {

        if (!std::ofstream(this->path())) // creates the file
            throw std::runtime_error("TemporaryHandle(): Could not create {" + this->str() + "}.");
    }

public:
    TemporaryHandle()                       = delete;
    TemporaryHandle(const TemporaryHandle&) = delete;
    TemporaryHandle(TemporaryHandle&&)      = default;

    // --- Construction ---
    // --------------------

    static TemporaryHandle create(std::filesystem::path path) {
        if (std::filesystem::exists(path))
            throw std::runtime_error("TemporaryHandle::create(): File {" + path.string() + "} already exists.");
        return TemporaryHandle(std::move(path));
    }

    static TemporaryHandle create() {
        const std::filesystem::path directory = std::filesystem::temp_directory_path();

        const auto random_path = [&] { return directory / std::filesystem::path(random_ascii_string()); };

        // Try generating random file names until unique one is found, effectively always happens
        // on the first attempt since the probability of a name collision is (1/25)^20 ~= 1e-28
        constexpr std::size_t max_attempts = 50;

        for (std::size_t i = 0; i < max_attempts; ++i) {
            std::filesystem::path path = random_path();

            if (std::filesystem::exists(path)) continue;

            return TemporaryHandle(std::move(path));
        }

        throw std::runtime_error("TemporaryHandle::create(): Could not create a unique filename.");
    }

    static TemporaryHandle overwrite(std::filesystem::path path) { return TemporaryHandle(std::move(path)); }

    static TemporaryHandle overwrite() {
        auto random_path = std::filesystem::temp_directory_path() / std::filesystem::path(random_ascii_string());
        return TemporaryHandle(std::move(random_path));
    }

    // --- Utils ---
    // -------------

    std::ifstream ifstream(std::ios::openmode mode = std::ios::in) const {
        std::ifstream file(this->path(), mode); // 'ifstream' always adds 'std::ios::in'
        if (!file) throw std::runtime_error("TemporaryHandle::ifstream() Could not open {" + this->str() + "}.");
        return file;
    }

    std::ofstream ofstream(std::ios::openmode mode = std::ios::out) const {
        std::ofstream file(this->path(), mode); // 'ofstream' always adds 'std::ios::out'
        if (!file) throw std::runtime_error("TemporaryHandle::ofstream() Could not open {" + this->str() + "}.");
        return file;
    }

    const std::filesystem::path& path() const noexcept { return this->filepath; }
    const std::string&           str() const noexcept { return this->string; }

    // --- Creation ---
    // ----------------

    ~TemporaryHandle() {
        if (!this->filepath.empty()) std::filesystem::remove(this->filepath);
    }
};

// ======================
// --- Shell commands ---
// ======================

// This seems the to be the fastest way of reading a text file
// into 'std::string' without invoking OS-specific methods
[[nodiscard]] inline std::string read_file_to_string(const std::string& path) {
    std::ifstream file(path, std::ios::ate); // open file and immediately seek to the end
    if (!file.good()) throw std::runtime_error("read_file_to_string(): Could not open file {" + path + ".");

    const auto file_size = file.tellg(); // returns cursor pos, which is the end of file
    file.seekg(std::ios::beg);           // seek to the beginning
    std::string chars(file_size, 0);     // allocate string of appropriate size
    file.read(chars.data(), file_size);  // read into the string
    return chars;
}

struct CommandResult {
    int         status; // aka error code
    std::string out;
    std::string err;
};

// A function to run shell command & capture it's status, stdout and stderr.
//

// Note 1:
// Creating temporary files doesn't seem to be ideal, but I'd yet to find
// a way to pipe BOTH stdout and stderr directly into the program without
// relying on platform-specific API like Unix forks and Windows processes
//
// Note 2:
// Usage of std::system() is often discouraged due to security reasons,
// but it doesn't seem there is a portable way to do better (aka going
// back to previous note about platform-specific APIs)
//
inline CommandResult run_command(std::string_view command) {
    const auto stdout_handle = TemporaryHandle::create();
    const auto stderr_handle = TemporaryHandle::create();
    
    constexpr std::string_view stdout_pipe_prefix = " >";
    constexpr std::string_view stderr_pipe_prefix = " 2>";

    // Run command while piping out/err to temporary files
    std::string pipe_command;
    pipe_command.reserve(command.size() + stdout_pipe_prefix.size() + stdout_handle.str().size() +
                         stderr_handle.str().size() + stderr_pipe_prefix.size() + 4);
    pipe_command += command;
    pipe_command += stdout_pipe_prefix;
    pipe_command += '"';
    pipe_command += stdout_handle.str();
    pipe_command += '"';
    pipe_command += stderr_pipe_prefix;
    pipe_command += '"';
    pipe_command += stderr_handle.str();
    pipe_command += '"';

    const int status = std::system(pipe_command.c_str());
    
    // Extract out/err from files
    std::string out = read_file_to_string(stdout_handle.str());
    std::string err = read_file_to_string(stderr_handle.str());
    
    // Remove possible LF/CRLF added by file piping at the end
    if (!out.empty() && out.back() == '\n') out.resize(out.size() - 1); // LF
    if (!out.empty() && out.back() == '\r') out.resize(out.size() - 1); // CR
    if (!err.empty() && err.back() == '\n') err.resize(err.size() - 1); // LF
    if (!err.empty() && err.back() == '\r') err.resize(err.size() - 1); // CR
    
    return {status, std::move(out), std::move(err)};
}

} // namespace utl::shell::impl

// ______________________ PUBLIC API ______________________

namespace utl::shell {

using impl::random_ascii_string;

using impl::TemporaryHandle;

using impl::CommandResult;
using impl::run_command;

} // namespace utl::shell

#endif
#endif // module utl::shell
