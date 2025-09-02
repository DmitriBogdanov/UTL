#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>

const std::filesystem::path directory = "temp/seed_correlation/";

template <class Gen>
void generate_binary_matrix(const std::filesystem::path& output_path) {
    std::cout << "Generating " << output_path << std::endl;

    constexpr std::size_t   w    = 200;
    constexpr std::size_t   h    = 160;
    
    using result_type = typename Gen::result_type;

    Gen gen;

    std::uniform_int_distribution dist{0, 1};

    std::ofstream out(output_path);

    for (std::size_t i = 0; i < h; ++i) {
        gen.seed(static_cast<result_type>(i));
        for (std::size_t j = 0; j < w; ++j) out << dist(gen) << (j + 1 == w ? '\n' : ',');
    }
}

int main() {
    std::filesystem::create_directories(directory); // ensures directory existence

    generate_binary_matrix<std::minstd_rand>(directory / "minstd_rand.csv");
    generate_binary_matrix<std::mt19937>(directory / "mt19937.csv");
}