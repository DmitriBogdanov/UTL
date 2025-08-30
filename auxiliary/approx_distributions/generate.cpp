#include "include/UTL/random.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

const std::filesystem::path directory = "temp/approx_distributions/";

template <class Dist>
void generate_values(const std::filesystem::path& output_path) {
    std::cout << "Generating " << output_path << std::endl;

    constexpr std::uint64_t seed = 3;
    constexpr std::size_t   N    = 1'000'000;
    // larger values result in a smoother empirical PDF, 1'000'000 results in ~10 MB of data

    std::ofstream out(output_path);

    utl::random::PRNG gen(seed);
    Dist              dist;

    for (std::size_t i = 0; i < N; ++i) out << dist(gen) << '\n';
}

int main() {
    std::filesystem::create_directories(directory); // ensures directory existence

    generate_values<utl::random::ApproxNormalDistribution<double>>(directory / "approx_normal.csv");
    generate_values<utl::random::NormalDistribution<double>>(directory / "precise_normal.csv");
}