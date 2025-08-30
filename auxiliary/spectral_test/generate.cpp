#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>

const std::filesystem::path directory = "temp/spectral_test/";

// Extremely low quality PRNG that fails a spectral test in 3D
// (aka generates discernible patterns when generating { RANDU(), RANDU(), RANDU() } triplets)
class RANDU {
public:
    using result_type = std::uint32_t;

private:
    result_type s{};

    constexpr static result_type factor = 65539;
    constexpr static result_type mod    = result_type(1) << 31;

public:
    constexpr explicit RANDU(result_type seed = 0) noexcept { this->seed(seed); }

    [[nodiscard]] static constexpr result_type min() noexcept { return 1; }
    [[nodiscard]] static constexpr result_type max() noexcept { return mod - 1; }

    constexpr void seed(result_type seed) noexcept { this->s = seed; }

    constexpr result_type operator()() noexcept { return this->s = factor * this->s % mod; }
};

template <class Gen>
void generate_points_in_cube(const std::filesystem::path& output_path) {
    std::cout << "Generating " << output_path << std::endl;
    
    using result_type = typename Gen::result_type;

    constexpr result_type seed   = 3;
    constexpr std::size_t points = 1000;
    constexpr result_type min    = Gen::min();
    constexpr result_type max    = Gen::max();

    Gen gen(seed);

    std::ofstream out(output_path);

    for (std::size_t i = 0; i < points; ++i) {
        const double x = static_cast<double>(gen() - min) / (max - min);
        const double y = static_cast<double>(gen() - min) / (max - min);
        const double z = static_cast<double>(gen() - min) / (max - min);

        out << x << ',' << y << ',' << z << '\n';
    }
}

int main() {
    std::filesystem::create_directories(directory); // ensures directory existence

    generate_points_in_cube<RANDU>(directory / "randu.csv");
}