#include "UTL/bit.hpp"
#include "UTL/enum_reflect.hpp"
#include "UTL/integral.hpp"
#include "UTL/json.hpp"
#include "UTL/log.hpp"
#include "UTL/math.hpp"
#include "UTL/mvl.hpp"
#include "UTL/parallel.hpp"
#include "UTL/predef.hpp"
#include "UTL/profiler.hpp"
#include "UTL/progressbar.hpp"
#include "UTL/random.hpp"
#include "UTL/shell.hpp"
#include "UTL/sleep.hpp"
#include "UTL/stre.hpp"
#include "UTL/struct_reflect.hpp"
#include "UTL/table.hpp"

#include <chrono>
#include <complex>
#include <string>
#include <utility>
#include <vector>

#define ANKERL_NANOBENCH_IMPLEMENT
#include "thirdparty/nanobench.h"

// Note:
// This is a common include for all benchmarks,
// it exists purely to reduce boilerplate and shouldn't be included anywhere else.

#define REPEAT(repeats_) for (int count_ = 0; count_ < repeats_; ++count_)
#define DO_NOT_OPTIMIZE_AWAY ankerl::nanobench::doNotOptimizeAway

inline ankerl::nanobench::Bench bench;

template <class Func>
void benchmark(const char* name, Func lambda) {
    bench.run(name, lambda);
}

template <class Func>
void benchmark(const std::string& name, Func lambda) {
    bench.run(name.c_str(), lambda);
}

using namespace utl;
using namespace std::chrono_literals;

// Set up fast random-stuff generation so we can benchmark on unpredictable workloads
// that can't be "folded" with optimization. The reason we don't just use 'utl::random'
// is because here we can select the fastest PRNG and do some stuff to generate strings faster.

namespace datagen {

// Generation params
constexpr std::size_t string_pregen_count = 500;

constexpr std::size_t min_string_size = 10;
constexpr std::size_t max_string_size = 120;

constexpr double min_double = -1e3;
constexpr double max_double = 1e3;

constexpr int min_int = -1500;
constexpr int max_int = 1500;

constexpr std::uint64_t min_uint = 15;
constexpr std::uint64_t max_uint = 17500;

// PRNG & buffers
inline random::generators::RomuDuoJr64 gen;
inline std::vector<std::string>        pregen_strings;
// we want random generation to be as fast as possible to reduce its impact on benchmarks that measure
// overhead of calling something else, which is why we choose the fastest PRNG available and pregenerate
// strings. Number generation is fast enough that we don't really get much benefit from pre-generation.

// Datagen functions
inline auto rand_bool() { return static_cast<bool>(random::UniformIntDistribution{0, 1}(gen)); }
inline auto rand_char() { return static_cast<char>(random::UniformIntDistribution{'a', 'z'}(gen)); }
inline auto rand_int() { return random::UniformIntDistribution{min_int, max_int}(gen); }
inline auto rand_uint() { return random::UniformIntDistribution{min_uint, max_uint}(gen); }
inline auto rand_double() { return random::UniformRealDistribution{min_double, max_double}(gen); }
inline auto rand_complex() { return std::complex{rand_double(), rand_double()}; }
inline auto rand_string() {
    return pregen_strings[random::UniformIntDistribution<std::size_t>{0, string_pregen_count - 1}(gen)];
}
inline auto rand_vector_of_strings() { return std::vector{rand_string(), rand_string(), rand_string(), rand_string()}; }

// Run some init logic before 'main()' to set up pregenerated values
struct _init_helper {
    _init_helper() {
        pregen_strings.resize(string_pregen_count);
        for (auto& e : pregen_strings)
            e = shell::random_ascii_string(random::uniform(min_string_size, max_string_size));
    }
};
inline _init_helper _init;

} // namespace datagen